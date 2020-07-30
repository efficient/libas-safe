use std::cell::Cell;

thread_local! {
	static EAN: Cell<bool> = Cell::default();
}

#[no_mangle]
extern fn timed_plugin() {
	assert!(!EAN.with(|ean| ean.get()));
	EAN.with(|ean| ean.replace(true));
	assert!(EAN.with(|ean| ean.get()));
}

#[no_mangle]
extern fn threaded_plugin() {
	assert!(EAN.with(|ean| ean.get()));
}
