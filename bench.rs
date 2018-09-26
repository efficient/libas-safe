#![feature(test)]

extern crate test;

mod dl;
mod dlfcn;
mod got;
mod mman;

use got::GOT;
use test::Bencher;

#[link(name = "lib")]
extern "C" {
	fn get() -> bool;
	fn set(_: bool);
}

#[bench]
fn base(lo: &mut Bencher) {
	lo.iter(|| GOT::new().unwrap());
}

#[bench]
fn clone_drop(lo: &mut Bencher) {
	let base = GOT::new().unwrap();
	lo.iter(|| base.clone());
}

fn main() {
	assert!(! unsafe {
		get()
	});
	unsafe {
		set(true);
	}
	assert!(unsafe {
		get()
	});

	let base = GOT::new().unwrap();
	let replace = base.clone().unwrap();
	unsafe {
		replace.install();
	}

	assert!(! unsafe {
		get()
	});
	unsafe {
		set(true);
	}
	assert!(unsafe {
		get()
	});
	unsafe {
		set(false);
	}
	assert!(! unsafe {
		get()
	});

	unsafe {
		base.install();
	}

	assert!(unsafe {
		get()
	});
}
