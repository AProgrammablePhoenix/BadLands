#include <cstddef>
#include <cstdint>

#include <shared/memory/defs.hpp>

#include <interrupts/Core.hpp>
#include <interrupts/idt.hpp>

struct IDTDescriptor {
	uint16_t offset0_15;
	uint16_t segmentSelector;
	uint16_t flags;
	uint16_t offset16_31;
	uint32_t offset32_63;
	uint32_t reserved;
};

enum class INTDPL {
	DPL0,
	DPL1,
	DPL2,
	DPL3
};

enum class INTTYPE {
	EXCEPTION,
	TRAP
};

namespace {
	static constexpr uint16_t FLAGS_PRESENT		    = 0x8000;

	static constexpr uint16_t FLAGS_DPL0		    = 0x0000;
	static constexpr uint16_t FLAGS_DPL1		    = 0x2000;
	static constexpr uint16_t FLAGS_DPL2		    = 0x4000;
	static constexpr uint16_t FLAGS_DPL3		    = 0x6000;

	static constexpr uint16_t FLAGS_EXCEPTION	    = 0x0E00;
	static constexpr uint16_t FLAGS_TRAP		    = 0x0F00;

	static constexpr uint16_t SS_CODE_KERNEL_GDT    = 0x0008;
	static constexpr uint16_t SS_CODE_USER_GDT	    = 0x0018;

    static constexpr size_t IDT_ENTRIES = 256;

	alignas(Shared::Memory::FRAME_SIZE) static IDTDescriptor IDT[256];

    static inline void registerCoreInterrupt(size_t intn, void(*_handler)(void), INTDPL dpl, INTTYPE type) {
		const uint64_t handler = reinterpret_cast<uint64_t>(_handler);

		IDTDescriptor* descriptor = &IDT[intn];
		descriptor->offset0_15 = static_cast<uint16_t>(handler);
		descriptor->segmentSelector = SS_CODE_KERNEL_GDT;

		descriptor->flags = FLAGS_PRESENT;
		switch (dpl) {
		default:
		case INTDPL::DPL0:
			descriptor->flags |= FLAGS_DPL0;
			break;
		case INTDPL::DPL1:
			descriptor->flags |= FLAGS_DPL1;
			break;
		case INTDPL::DPL2:
			descriptor->flags |= FLAGS_DPL2;
			break;
		case INTDPL::DPL3:
			descriptor->flags |= FLAGS_DPL3;
			break;
		}
		switch (type) {
		default:
		case INTTYPE::EXCEPTION:
			descriptor->flags |= FLAGS_EXCEPTION;
			break;
		case INTTYPE::TRAP:
			descriptor->flags |= FLAGS_TRAP;
			break;
		}

		descriptor->offset16_31 = static_cast<uint16_t>(handler >> 16);
		descriptor->offset32_63 = static_cast<uint32_t>(handler >> 32);
		descriptor->reserved = 0;
	}
}

void Interrupts::kernel_idt_setup() {
 	// set up each core exception handler
// 	registerCoreInterrupt(0,  &Interrupts::Core::int_divide_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(1,  &Interrupts::Core::int_debug_trap,				INTDPL::DPL0, INTTYPE::TRAP);
// 	registerCoreInterrupt(2,  &Interrupts::Core::int_nmi_error,					INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(3,  &Interrupts::Core::int_breakpoint_trap,			INTDPL::DPL0, INTTYPE::TRAP);
// 	registerCoreInterrupt(4,  &Interrupts::Core::int_overflow_trap,				INTDPL::DPL0, INTTYPE::TRAP);
// 	registerCoreInterrupt(5,  &Interrupts::Core::int_bound_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(6,  &Interrupts::Core::int_invalidop_error,			INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(7,  &Interrupts::Core::int_device_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(8,  &Interrupts::Core::int_doublefault_error,			INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(9,  &Interrupts::Core::int_coprocseg_error,			INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(10, &Interrupts::Core::int_invalidtss_error,			INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(11, &Interrupts::Core::int_segpresence_error,			INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(12, &Interrupts::Core::int_stack_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(13, &Interrupts::Core::int_gp_error,					INTDPL::DPL0, INTTYPE::EXCEPTION);
 	registerCoreInterrupt(14, &Interrupts::Core::int_page_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(16, &Interrupts::Core::int_x87fp_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(17, &Interrupts::Core::int_align_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(18, &Interrupts::Core::int_machine_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(19, &Interrupts::Core::int_simd_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(20, &Interrupts::Core::int_virt_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(21, &Interrupts::Core::int_controlprotection_error,	INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(28, &Interrupts::Core::int_hypervirt_error,			INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(29, &Interrupts::Core::int_vmmcom_error,				INTDPL::DPL0, INTTYPE::EXCEPTION);
// 	registerCoreInterrupt(30, &Interrupts::Core::int_security_error,			INTDPL::DPL0, INTTYPE::EXCEPTION);

	uint8_t IDTP[10];
	*reinterpret_cast<uint16_t*>(IDTP) = sizeof(IDT) - 1;
	*reinterpret_cast<IDTDescriptor**>(IDTP + sizeof(uint16_t)) = IDT;

	__asm__ volatile("lidt (%0)" :: "memory"(IDTP));
}
