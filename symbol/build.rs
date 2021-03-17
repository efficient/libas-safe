use std::env::var;
use std::process::Command;

fn main() {
	let out_dir = format!("target/{}/deps", var("PROFILE").unwrap());
	assert!(Command::new("cp").arg("libgotcha/libgotcha.so").arg(&out_dir).status().unwrap().success());
	println!("cargo:rustc-link-search={}", out_dir);
}
