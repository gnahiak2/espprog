use clap::Parser;
use serialport::SerialPort;
use std::fs::File;
use std::io::{Write, BufRead, BufReader};
use std::time:Duration;

const BUF_SIZE: usize = 256;

#[derive(Parser)]
struct Args {
    #[arg(short, long)]
    port: String,

    #[arg(short, long)]
    output: String,

}

fn