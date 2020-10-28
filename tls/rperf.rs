const ITERS: u32 = 2500000;

fn main() {
	extern {
		fn __call_tls_dtors();
		fn _dl_allocate_tls(_: usize) -> usize;
		fn _dl_deallocate_tls(_: usize, _: bool);
	}

	let nsthen = nsnow();
	for _ in 0..ITERS {
		unsafe {
			__call_tls_dtors();
			_dl_deallocate_tls(_dl_allocate_tls(0), true);
		}
	}

	let nswhen = (nsnow() - nsthen) / u64::from(ITERS);
	println!("{}.{:.03} μs", nswhen / 1_000, nswhen % 1_000);
}

fn nsnow() -> u64 {
	const CLOCK_REALTIME: u32 = 0;
	extern {
		fn clock_gettime(_: u32, _: &mut Tv);
	}

	#[repr(C)]
	struct Tv {
		tv_sec: u64,
		tv_nsec: u64,
	}

	let mut tv = Tv {
		tv_sec: 0,
		tv_nsec: 0,
	};
	unsafe {
		clock_gettime(CLOCK_REALTIME, &mut tv);
	}
	tv.tv_sec * 1_000_000_000 + tv.tv_nsec
}
