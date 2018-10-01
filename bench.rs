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
fn clone_drop(lo: &mut Bencher) {
	lo.iter(|| GOT::new().unwrap());
}

#[bench]
fn install_uninstall(lo: &mut Bencher) {
	let mut got = Some(GOT::new().unwrap());
	lo.iter(|| if let Some(got) = got.take() {
		unsafe {
			got.install();
		}
	} else {
		got = unsafe {
			GOT::uninstall()
		};
	});
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

	unsafe {
		GOT::new().unwrap().install();
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
		GOT::uninstall();
	}

	assert!(unsafe {
		get()
	});
}
