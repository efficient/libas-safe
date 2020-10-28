#![feature(test)]

extern crate test;
use test::Bencher;

#[bench]
fn tls_alloc_dealloc(lo: &mut Bencher) {
	extern {
		fn _dl_allocate_tls(_: usize) -> usize;
		fn _dl_deallocate_tls(_: usize, _: bool);
	}

	lo.iter(|| unsafe {
		_dl_deallocate_tls(_dl_allocate_tls(0), true)
	});
}
