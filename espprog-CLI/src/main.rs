// main.rs
use clap::{Parser, Subcommand};
use serialport::SerialPort;
use std::fs::File;
use std::io::{BufWriter, Read, Write};
use std::time::{Duration, Instant};

#[derive(Parser)]
struct Cli {
    #[arg(short, long)]
    port: String,
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    Probe,
    Read { output: String },
    Write { input: String },
}

fn open_port(name: &str) -> Box<dyn SerialPort> {
    serialport::new(name, 921600)
        .timeout(Duration::from_secs(10))
        .open()
        .unwrap()
}

fn read_line(port: &mut Box<dyn SerialPort>) -> String {
    let mut out = Vec::new();
    let mut b = [0u8; 1];
    loop {
        port.read_exact(&mut b).unwrap();
        if b[0] == b'\n' { break; }
        out.push(b[0]);
    }
    String::from_utf8(out).unwrap()
}

fn probe_flash(port: &mut Box<dyn SerialPort>) -> (usize, String) {
    port.write_all(b"PROBE\n").unwrap();
    let line = read_line(port);
    let parts: Vec<&str> = line.trim().split_whitespace().collect();
    if parts.len() != 3 || parts[0] != "OK" {
        panic!("Unexpected response: {}", line);
    }
    (parts[1].parse().unwrap(), parts[2].to_string())
}

fn main() {
    let cli = Cli::parse();
    let mut port = open_port(&cli.port);

    match cli.command {
        Commands::Probe => {
            let (size, mode) = probe_flash(&mut port);
            println!("Flash size: {} bytes ({:.2} MB)", size, size as f64 / 1024.0 / 1024.0);
            println!("Mode: {}", mode);
        }
        Commands::Read { output } => {
            let (size, mode) = probe_flash(&mut port);
            println!("Reading {:.2} MB using {}...", size as f64 / 1024.0 / 1024.0, mode);

            port.write_all(b"READ\n").unwrap();

            let file = File::create(&output).unwrap();
            let mut file = BufWriter::with_capacity(1024 * 1024, file);

            let mut buf = vec![0u8; 65536];
            let mut got = 0usize;
            let start = Instant::now();

            while got < size {
                let n = port.read(&mut buf).unwrap();
                file.write_all(&buf[..n]).unwrap();
                got += n;
                print!("\r{:6.2}% ({}/{})", (got as f64 / size as f64) * 100.0, got, size);
                std::io::stdout().flush().unwrap();
            }

            file.flush().unwrap();
            let sec = start.elapsed().as_secs_f64();
            println!("\nCompleted in {:.2}s", sec);
            println!("Speed: {:.2} MB/s", (got as f64 / 1024.0 / 1024.0) / sec);
        }
        Commands::Write { input } => {
            let data = std::fs::read(&input).unwrap();
            let cmd = format!("WRITE:{}\n", data.len());
            port.write_all(cmd.as_bytes()).unwrap();

            let ready = read_line(&mut port);
            if ready.trim() != "READY" {
                panic!("Unexpected response: {}", ready);
            }

            let start = Instant::now();
            for chunk in data.chunks(65536) {
                port.write_all(chunk).unwrap();
            }

            let sec = start.elapsed().as_secs_f64();
            println!("Write complete ({:.2} MB/s)", (data.len() as f64 / 1024.0 / 1024.0) / sec);
        }
    }
}