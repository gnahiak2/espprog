use clap::{Parser, Subcommand};

use serialport::SerialPort;

use std::fs::File;
use std::io::{Read, Write};
use std::time::Duration;

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

    Read {
        output: String,
    },

    Write {
        input: String,
    },
}

fn open_port(name: &str)
-> Box<dyn SerialPort>
{
    serialport::new(name, 921600)
        .timeout(Duration::from_secs(2))
        .open()
        .unwrap()
}

fn main()
{
    let cli = Cli::parse();

    let mut port = open_port(&cli.port);

    match cli.command
    {
        Commands::Probe =>
        {
            port.write_all(b"PROBE\n").unwrap();

            let mut buf = [0u8; 256];

            let n = port.read(&mut buf).unwrap();

            println!(
                "{}",
                String::from_utf8_lossy(&buf[..n])
            );
        }

        Commands::Read { output } =>
        {
            port.write_all(b"READ\n").unwrap();

            let mut file =
                File::create(output).unwrap();

            let mut buf = [0u8; 256];

            loop
            {
                let n = port.read(&mut buf).unwrap();

                if &buf[..5.min(n)] == b"DONE\n" {
                    break;
                }

                file.write_all(&buf[..n]).unwrap();
            }

            println!("Read complete");
        }

        Commands::Write { input } =>
        {
            let data =
                std::fs::read(&input).unwrap();

            let cmd =
                format!("WRITE:{}\n", data.len());

            port.write_all(cmd.as_bytes()).unwrap();

            let mut ready = [0u8; 6];

            port.read_exact(&mut ready).unwrap();

            if &ready != b"READY\n" {
                panic!("Device not ready");
            }

            for chunk in data.chunks(256)
            {
                port.write_all(chunk).unwrap();
            }

            println!("Write complete");
        }
    }
}