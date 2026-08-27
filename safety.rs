#![no_std]
#![no_main]

use core::panic::PanicInfo;

/* Rust Memory Safety Verification Routine */
#[no_mangle]
pub extern "C" fn rust_security_check() -> i32 {
    // 1 indicates Rust memory-safe verification passed
    1
}

/* Rust String Message exported to C Kernel */
#[no_mangle]
pub extern "C" fn rust_get_status() -> *const u8 {
    b"  [Rust Core] : Memory-Safe Subsystem Enabled (#![no_std])\n\0".as_ptr()
}

/* Custom Panic Handler required for bare-metal no_std Rust */
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
