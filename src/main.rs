use std::{
    collections::HashMap,
    fmt::{Debug, Display},
    sync::{Arc, Mutex},
};

use clap::Parser;

use local_ip_address::local_ip;
use nix::{
    errno::Errno,
    fcntl::OFlag,
    sched::{setns, CloneFlags},
    sys::stat::Mode,
};
use warp::Filter;

use wasmedge_sdk::{
    config::{CommonConfigOptions, ConfigBuilder, HostRegistrationConfigOptions},
    params, Vm,
};

/// A simple http server that runs wasm functions.
#[derive(Parser, Debug)]
#[command(author, version, about, long_about)]
struct Args {
    /// Network namespace.
    #[arg(short, long)]
    netns: String,

    /// The path to the wasm module.
    #[arg(short, long)]
    mod_path: String,

    /// The default commandline arguments.
    #[arg(short, long, default_value_t = String::from(""))]
    args: String,

    /// The environment variables in the format ENV_VAR_NAME=VALUE.
    #[arg(short, long, default_value_t = String::from(""))]
    envs: String,

    /// The directories to pre-open. The required format is DIR1:DIR2.
    #[arg(short, long, default_value_t = String::from(""))]
    preopens: String,
}

pub enum RunError {
    // WasmEdge(Box<WasmEdgeError>),
    // IO(std::io::Error),
    // NoRootInSpec,
    Sys(Errno),
}

impl Display for RunError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            RunError::Sys(errno) => {
                write!(f, "system error, errno: {}", errno)
            }
        }
    }
}

impl Debug for RunError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            RunError::Sys(errno) => {
                write!(f, "system error, errno: {}", errno)
            }
        }
    }
}

impl std::error::Error for RunError {}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args = Args::parse();

    let netns = &*args.netns;
    println!("using netns: {}", netns);

    if !netns.is_empty() {
        let netns_fd =
            nix::fcntl::open(netns, OFlag::O_CLOEXEC, Mode::empty()).map_err(RunError::Sys)?;
        setns(netns_fd, CloneFlags::CLONE_NEWNET).map_err(RunError::Sys)?;
    }

    println!("using mod_path: {}", args.mod_path);

    let ip = local_ip().unwrap();
    println!("ip: {}", ip);

    // create a config with the `wasi` option enabled
    let config = ConfigBuilder::new(CommonConfigOptions::default())
        .with_host_registration_config(HostRegistrationConfigOptions::default().wasi(true))
        .build()
        .unwrap();
    assert!(config.wasi_enabled());

    // create a VM with the config
    let vm = Vm::new(Some(config)).unwrap();

    let vm = Arc::new(Mutex::new(vm));

    let func = warp::path!("func").and(warp::body::json()).map(
        move |params: HashMap<String, Vec<String>>| -> String {
            let mut vm = vm.lock().unwrap();
            match params.get("params") {
                Some(params) => {
                    let args: Vec<&str> = params.iter().map(|s| s.as_str()).collect();
                    vm.wasi_module().expect("Not found wasi module").initialize(
                        Some(args),
                        None,
                        None,
                    );
                }
                _ => {
                    vm.wasi_module()
                        .expect("Not found wasi module")
                        .initialize(None, None, None);
                }
            };
            match vm.run_func_from_file(&args.mod_path, "_start", params!()) {
                Ok(_) => "0".to_string(),
                Err(_) => "1".to_string(),
            }
        },
    );

    println!("[---youtirsin---] running warp server.");
    warp::serve(func).run(([127, 0, 0, 1], 10086)).await;

    Ok(())
}
