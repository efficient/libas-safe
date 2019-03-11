#![feature(test)]

extern crate globl;
extern crate test;

use globl::DATA;
use globl::RODATA;
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
