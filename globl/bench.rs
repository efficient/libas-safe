#![feature(test)]

extern crate globl;
extern crate proxy;
extern crate test;

use globl::DATA;
use globl::RODATA;
use proxy::proxy;
use test::Bencher;

#[bench]
fn rodata(lo: &mut Bencher) {
	#[inline(never)]
	fn rodata() -> bool { RODATA }

	lo.iter(rodata);
}

#[bench]
fn data(lo: &mut Bencher) {
	#[inline(never)]
	fn data() -> bool { unsafe {
		DATA
	}}

	lo.iter(data);
}

#[bench]
fn rodata_location(lo: &mut Bencher) {
	let rodata_location = |_| &RODATA;

	lo.iter(|| *proxy(0, rodata_location, ()).unwrap());
}

#[bench]
fn data_location(lo: &mut Bencher) {
	let data_location = |_| unsafe {
		&DATA
	};

	lo.iter(|| *proxy(0, data_location, ()).unwrap());
}
