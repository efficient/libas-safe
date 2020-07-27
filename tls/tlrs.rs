extern crate inger;

use std::cell::Cell;
use std::io::Result;

fn timed() {
	use inger::pause;

	thread_local! {
		static EAN: Cell<bool> = Cell::from(false);
	}

	let get = |ean: &Cell<_>| ean.get();
	assert!(!EAN.with(get));
	EAN.with(|ean| ean.replace(true));
	assert!(EAN.with(get));
	pause();
	assert!(EAN.with(get));
}

fn main() -> Result<()> {
	use inger::launch;
	use inger::resume;
	use std::thread::spawn;

	let mut state = launch(timed, u64::max_value())?;
	let nation = spawn(move || drop(resume(&mut state, u64::max_value())));
	drop(nation.join());

	Ok(())
}
