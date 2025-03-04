#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sched/smp.h>

#define PAGE_SIZE 0x1000ull
#define KERNEL_HIGH_VMA 0xffffffff80000000

extern uint64_t HIGH_VMA;

#define MSR_LAPIC_BASE 0x1b
#define MSR_EFER 0xc0000080
#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_CSTAR 0xc0000083
#define MSR_SFMASK 0xc0000084
#define PAT_MSR 0x277

#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101
#define KERNEL_GS_BASE 0xc0000102

#define MSR_HW_FEEDBACK_PTR 0x17d0
#define MSR_HW_FEEDBACK_CONFIG 0x17d1

#define MSR_PACKAGE_THERM_STATUS 0x1b1
#define MSR_PACKAGE_THERM_INTERRUPT 0x1b2

#define COM1 0x3f8
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

#define CORE_LOCAL ({ \
	(struct cpu_local*)(rdmsr(MSR_GS_BASE)); \
})

struct registers {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t isr_number;
	uint64_t error_code;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

struct cpuid_state {
	uint64_t leaf;
	uint64_t subleaf;
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
};

static inline void outb(uint16_t port, uint8_t data) {
	asm volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t data) {
	asm volatile("outw %0, %1" :: "a"(data), "Nd"(port));
}

static inline void outd(uint16_t port, uint32_t data) {
	asm volatile("outl %0, %1" :: "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t data;
	asm volatile("inb %1, %0" : "=a"(data):"Nd"(port));
	return data;
}

static inline uint16_t inw(uint16_t port) {
	uint16_t data;
	asm volatile("inw %1, %0" : "=a"(data):"Nd"(port));
	return data;
}

static inline uint32_t ind(uint16_t port) {
	uint32_t data;
	asm volatile("inl %1, %0" : "=a"(data):"Nd"(port));
	return data;
}

static inline uint64_t rdmsr(uint32_t msr) {
	uint64_t rax, rdx;
	asm volatile ("rdmsr" : "=a"(rax), "=d"(rdx) : "c"(msr) : "memory");
	return (rdx << 32) | rax;
}

static inline void wrmsr(uint32_t msr, uint64_t data) {
	uint64_t rax = (uint32_t)data;
	uint64_t rdx = data >> 32;
	asm volatile ("wrmsr" :: "a"(rax), "d"(rdx), "c"(msr));
}

static inline void swapgs(void) {
	asm volatile ("swapgs" ::: "memory");
}

static inline void set_kernel_gs(uintptr_t addr) {
	wrmsr(MSR_GS_BASE, addr);
}

static inline void set_user_gs(uintptr_t addr) {
	wrmsr(KERNEL_GS_BASE, addr);
}

static inline uint64_t get_user_gs() {
	return rdmsr(KERNEL_GS_BASE);
}

static inline void set_user_fs(uint64_t addr) {
	wrmsr(MSR_FS_BASE, addr);
}

static inline uint64_t get_user_fs() {
	return rdmsr(MSR_FS_BASE);
}

static inline void invlpg(uint64_t vaddr) {
	asm volatile ("invlpg %0" :: "m"((*((int(*)[])((void*)vaddr)))) : "memory");
}

static inline void set_errno(uint64_t code) {
	CORE_LOCAL->errno = code;
}

static inline uint64_t get_errno() {
	return CORE_LOCAL->errno;
}

struct cpuid_state cpuid(size_t leaf, size_t subleaf);
bool get_interrupt_state();
void init_cpu_features();

static inline bool bsfl(int data, int *position) {
	bool ret;
	asm volatile("bsf %1, %2" : "=@ccnz"(ret) : "r"(data), "r"(*position));
	return ret;
}
