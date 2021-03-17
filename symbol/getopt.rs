extern crate gotcha;
use libc;

use gotcha::Group;
use gotcha::group_lookup_symbol;
use gotcha::group_lookup_symbol_mut;
use gotcha::group_thread_set;
use libc::getopt;
use std::convert::TryInto;
use std::env::args;
use std::ffi::CString;
use std::os::raw::c_char;
use std::os::raw::c_int;
use std::ptr::eq;
extern {
	static optarg: *const c_char;
	static optind: c_int;
	static mut opterr: c_int;
	static optopt: c_int;
}

fn main() -> Result<(), &'static str> {
	let mut args: Vec<_> = args().collect();
	if args.len() == 1 {
		Err("missing optstring argument")?;
	}

	let arg0 = args.remove(0);
	let optstring = CString::new(format!(":{}", args[0])).unwrap();
	let optstring = optstring.as_ptr();
	args[0] = arg0.clone();

	let mut args: Vec<_> = args.into_iter().map(|arg| CString::new(arg).unwrap().into_raw()).collect();
	let mut option;
	while {
		option = unsafe {
			getopts(&mut args, optstring)
		};
		option != -1
	} {
		match option.try_into().unwrap() {
		b'?' =>
			eprintln!("{}: invalid option -- '{}'", arg0, unsafe {
				optopt
			}),
		b':' =>
			eprintln!("{}: option requires an argument -- '{}'", arg0, unsafe {
				optopt
			}),
		_ =>
			if ! unsafe {
				optarg
			}.is_null() {
				assert!(unsafe {
					optarg
				} == args[optindex() - 1]);
			}
		}
	}

	let args: Vec<_> = args.into_iter().map(|arg| unsafe {
		CString::from_raw(arg)
	}.into_string().unwrap()).collect();
	for arg in &args[1..optindex()] {
		print!(" {}", arg);
	}
	print!(" --");
	for arg in &args[optindex()..] {
		print!(" {}", arg);
	}
	println!();

	Ok(())
}

unsafe fn getopts(args: &mut [*mut c_char], optstring: *const c_char) -> c_int {
	struct Opts {
		group: Group,
		optarg1: *const *const c_char,
		optind1: *const c_int,
		opterr1: *mut c_int,
		optopt1: *const c_int,
	}

	thread_local! {
		static OPTS: Opts = unsafe {
			let group = Group::new().unwrap();
			assert!(! group.is_shared());

			let optarg1 = group_lookup_symbol!(group, optarg).unwrap();
			let optind1 = group_lookup_symbol!(group, optind).unwrap();
			let opterr1 = group_lookup_symbol_mut!(group, opterr).unwrap();
			let optopt1 = group_lookup_symbol!(group, optopt).unwrap();

			assert!(! eq(optarg1, &optarg));
			assert!(! eq(optind1, &optind));
			assert!(! eq(&*opterr1, &opterr));
			assert!(! eq(optopt1, &optopt));

			opterr = 0;
			*opterr1 = 0;

			Opts {
				group,
				optarg1,
				optind1,
				opterr1,
				optopt1,
			}
		};
	}
	OPTS.with(|Opts { group, optarg1, optind1, opterr1, optopt1 }| {
		let mut args1: Vec<_> = args.iter().copied().collect();

		assert!(optarg == **optarg1);
		assert!(optind == **optind1);
		assert!(opterr == **opterr1);
		assert!(optopt == **optopt1);

		let res = getopt(args.len().try_into().unwrap(), args.as_mut_ptr(), optstring);
		group_thread_set!(*group);

		let res1 = getopt(args1.len().try_into().unwrap(), args1.as_mut_ptr(), optstring);
		group_thread_set!(Group::SHARED);

		assert!(res == res1);
		assert!(optarg == **optarg1);
		assert!(optind == **optind1);
		assert!(opterr == **opterr1);
		assert!(optopt == **optopt1);

		res
	})
}

#[inline]
fn optindex() -> usize {
	unsafe {
		optind
	}.try_into().unwrap()
}
