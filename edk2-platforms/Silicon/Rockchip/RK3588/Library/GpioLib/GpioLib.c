#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/GpioLib.h>

#define GRF_GPIO_IOMUX_REG(Pin)         (((Pin) / 4) * 4)
#define GRF_GPIO_IOMUX_SHIFT(Pin)       (((Pin) % 4) * 4)
#define GRF_GPIO_IOMUX_MASK(Pin)        (0xFU << (GRF_GPIO_IOMUX_SHIFT(Pin) + 16))

#define GRF_GPIO_P_REG(Pin)             (((Pin) / 8) * 4)
#define GRF_GPIO_P_SHIFT(Pin)           (((Pin) % 8) * 2)
#define GRF_GPIO_P_MASK(Pin)            (0x3U << (GRF_GPIO_P_SHIFT (Pin) + 16))

#define GRF_GPIO_DS_REG(Pin)            (((Pin) / 2) * 4)
#define GRF_GPIO_DS_SHIFT(Pin)          (((Pin) % 2) * 8)
#define GRF_GPIO_DS_MASK(Pin)           (0xFFU << (GRF_GPIO_DS_SHIFT (Pin) + 16))

#define GRF_GPIO_IE_REG(Pin)            (((Pin) / 8) * 4)
#define GRF_GPIO_IE_SHIFT(Pin)          (((Pin) % 8) * 2)
#define GRF_GPIO_IE_MASK(Pin)           (0x3U << (GRF_GPIO_IE_SHIFT (Pin) + 16))

#define GPIO_SWPORT_DR(Pin)             ((Pin) < 16 ? 0x0000 : 0x0004)
#define GPIO_SWPORT_DDR(Pin)            ((Pin) < 16 ? 0x0008 : 0x000C)

#define GPIO_WRITE_MASK(Pin)            (1U << (((Pin) % 16) + 16))
#define GPIO_VALUE_MASK(Pin, Value)     ((UINT32)Value << ((Pin) % 16))

#define GPIO_NGROUPS                    5

#define PMU1_IOC_BASE     (0xFD5F0000U)
#define PMU2_IOC_BASE     (PMU1_IOC_BASE + 0x4000)
#define BUS_IOC_BASE      (PMU1_IOC_BASE + 0x8000)
#define VCCIO1_4_IOC_BASE (PMU1_IOC_BASE + 0x9000)
#define VCCIO3_5_IOC_BASE (PMU1_IOC_BASE + 0xA000)
#define VCCIO2_IOC_BASE   (PMU1_IOC_BASE + 0xB000)
#define VCCIO6_IOC_BASE   (PMU1_IOC_BASE + 0xC000)
#define EMMC_IOC_BASE     (PMU1_IOC_BASE + 0xD000)

typedef struct {
  UINT8 Group;
  UINT8 Pin;
  EFI_PHYSICAL_ADDRESS Addr;
} GPIO_REG;

STATIC GPIO_REG mDsReg[] = {
  {0, GPIO_PIN_PA0, PMU1_IOC_BASE + 0x0010},
	{0, GPIO_PIN_PA4, PMU1_IOC_BASE + 0x0014},
	{0, GPIO_PIN_PB0, PMU1_IOC_BASE + 0x0018},
	{0, GPIO_PIN_PB4, PMU2_IOC_BASE + 0x0014},
	{0, GPIO_PIN_PC0, PMU2_IOC_BASE + 0x0018},
	{0, GPIO_PIN_PC4, PMU2_IOC_BASE + 0x001C},
	{0, GPIO_PIN_PD0, PMU2_IOC_BASE + 0x0020},
	{0, GPIO_PIN_PD4, PMU2_IOC_BASE + 0x0024},
	{1, GPIO_PIN_PA0, VCCIO1_4_IOC_BASE + 0x0020},
	{1, GPIO_PIN_PA4, VCCIO1_4_IOC_BASE + 0x0024},
	{1, GPIO_PIN_PB0, VCCIO1_4_IOC_BASE + 0x0028},
	{1, GPIO_PIN_PB4, VCCIO1_4_IOC_BASE + 0x002C},
	{1, GPIO_PIN_PC0, VCCIO1_4_IOC_BASE + 0x0030},
	{1, GPIO_PIN_PC4, VCCIO1_4_IOC_BASE + 0x0034},
	{1, GPIO_PIN_PD0, VCCIO1_4_IOC_BASE + 0x0038},
	{1, GPIO_PIN_PD4, VCCIO1_4_IOC_BASE + 0x003C},
	{2, GPIO_PIN_PA0, EMMC_IOC_BASE + 0x0040},
	{2, GPIO_PIN_PA4, VCCIO3_5_IOC_BASE + 0x0044},
	{2, GPIO_PIN_PB0, VCCIO3_5_IOC_BASE + 0x0048},
	{2, GPIO_PIN_PB4, VCCIO3_5_IOC_BASE + 0x004C},
	{2, GPIO_PIN_PC0, VCCIO3_5_IOC_BASE + 0x0050},
	{2, GPIO_PIN_PC4, VCCIO3_5_IOC_BASE + 0x0054},
	{2, GPIO_PIN_PD0, EMMC_IOC_BASE + 0x0058},
	{2, GPIO_PIN_PD4, EMMC_IOC_BASE + 0x005C},
	{3, GPIO_PIN_PA0, VCCIO3_5_IOC_BASE + 0x0060},
	{3, GPIO_PIN_PA4, VCCIO3_5_IOC_BASE + 0x0064},
	{3, GPIO_PIN_PB0, VCCIO3_5_IOC_BASE + 0x0068},
	{3, GPIO_PIN_PB4, VCCIO3_5_IOC_BASE + 0x006C},
	{3, GPIO_PIN_PC0, VCCIO3_5_IOC_BASE + 0x0070},
	{3, GPIO_PIN_PC4, VCCIO3_5_IOC_BASE + 0x0074},
	{3, GPIO_PIN_PD0, VCCIO3_5_IOC_BASE + 0x0078},
	{3, GPIO_PIN_PD4, VCCIO3_5_IOC_BASE + 0x007C},
	{4, GPIO_PIN_PA0, VCCIO6_IOC_BASE + 0x0080},
	{4, GPIO_PIN_PA4, VCCIO6_IOC_BASE + 0x0084},
	{4, GPIO_PIN_PB0, VCCIO6_IOC_BASE + 0x0088},
	{4, GPIO_PIN_PB4, VCCIO6_IOC_BASE + 0x008C},
	{4, GPIO_PIN_PC0, VCCIO6_IOC_BASE + 0x0090},
	{4, GPIO_PIN_PC2, VCCIO3_5_IOC_BASE + 0x0090},
	{4, GPIO_PIN_PC4, VCCIO3_5_IOC_BASE + 0x0094},
	{4, GPIO_PIN_PD0, VCCIO2_IOC_BASE + 0x0098},
  {0, 0, 0} // Terminator
};

STATIC GPIO_REG mPullReg[] = {
  {0, GPIO_PIN_PA0, PMU1_IOC_BASE + 0x0020},
	{0, GPIO_PIN_PB0, PMU1_IOC_BASE + 0x0024},
	{0, GPIO_PIN_PB5, PMU2_IOC_BASE + 0x0028},
	{0, GPIO_PIN_PC0, PMU2_IOC_BASE + 0x002C},
	{0, GPIO_PIN_PD0, PMU2_IOC_BASE + 0x0030},
	{1, GPIO_PIN_PA0, VCCIO1_4_IOC_BASE + 0x0110},
	{1, GPIO_PIN_PB0, VCCIO1_4_IOC_BASE + 0x0114},
	{1, GPIO_PIN_PC0, VCCIO1_4_IOC_BASE + 0x0118},
	{1, GPIO_PIN_PD0, VCCIO1_4_IOC_BASE + 0x011C},
	{2, GPIO_PIN_PA0, EMMC_IOC_BASE + 0x0120},
	{2, GPIO_PIN_PA4, VCCIO3_5_IOC_BASE + 0x0120},
	{2, GPIO_PIN_PB0, VCCIO3_5_IOC_BASE + 0x0124},
	{2, GPIO_PIN_PC0, VCCIO3_5_IOC_BASE + 0x0128},
	{2, GPIO_PIN_PD0, EMMC_IOC_BASE + 0x012C},
	{3, GPIO_PIN_PA0, VCCIO3_5_IOC_BASE + 0x0130},
	{3, GPIO_PIN_PB0, VCCIO3_5_IOC_BASE + 0x0134},
	{3, GPIO_PIN_PC0, VCCIO3_5_IOC_BASE + 0x0138},
	{3, GPIO_PIN_PD0, VCCIO3_5_IOC_BASE + 0x013C},
	{4, GPIO_PIN_PA0, VCCIO6_IOC_BASE + 0x0140},
	{4, GPIO_PIN_PB0, VCCIO6_IOC_BASE + 0x0144},
	{4, GPIO_PIN_PC0, VCCIO6_IOC_BASE + 0x0148},
	{4, GPIO_PIN_PC2, VCCIO3_5_IOC_BASE + 0x0148},
	{4, GPIO_PIN_PD0, VCCIO2_IOC_BASE + 0x014C},
  {0, 0, 0} // Terminator
};

STATIC GPIO_REG mSmtReg[] = {
  {0, GPIO_PIN_PA0, PMU1_IOC_BASE + 0x0030},
	{0, GPIO_PIN_PB0, PMU1_IOC_BASE + 0x0034},
	{0, GPIO_PIN_PB5, PMU2_IOC_BASE + 0x0040},
	{0, GPIO_PIN_PC0, PMU2_IOC_BASE + 0x0044},
	{0, GPIO_PIN_PD0, PMU2_IOC_BASE + 0x0048},
	{1, GPIO_PIN_PA0, VCCIO1_4_IOC_BASE + 0x0210},
	{1, GPIO_PIN_PB0, VCCIO1_4_IOC_BASE + 0x0214},
	{1, GPIO_PIN_PC0, VCCIO1_4_IOC_BASE + 0x0218},
	{1, GPIO_PIN_PD0, VCCIO1_4_IOC_BASE + 0x021C},
	{2, GPIO_PIN_PA0, EMMC_IOC_BASE + 0x0220},
	{2, GPIO_PIN_PA4, VCCIO3_5_IOC_BASE + 0x0220},
	{2, GPIO_PIN_PB0, VCCIO3_5_IOC_BASE + 0x0224},
	{2, GPIO_PIN_PC0, VCCIO3_5_IOC_BASE + 0x0228},
	{2, GPIO_PIN_PD0, EMMC_IOC_BASE + 0x022C},
	{3, GPIO_PIN_PA0, VCCIO3_5_IOC_BASE + 0x0230},
	{3, GPIO_PIN_PB0, VCCIO3_5_IOC_BASE + 0x0234},
	{3, GPIO_PIN_PC0, VCCIO3_5_IOC_BASE + 0x0238},
	{3, GPIO_PIN_PD0, VCCIO3_5_IOC_BASE + 0x023C},
	{4, GPIO_PIN_PA0, VCCIO6_IOC_BASE + 0x0240},
	{4, GPIO_PIN_PB0, VCCIO6_IOC_BASE + 0x0244},
	{4, GPIO_PIN_PC0, VCCIO6_IOC_BASE + 0x0248},
	{4, GPIO_PIN_PC2, VCCIO3_5_IOC_BASE + 0x0248},
	{4, GPIO_PIN_PD0, VCCIO2_IOC_BASE + 0x024C},
  {0, 0, 0} // Terminator
};

#define GPIO_BASE(n)        ((n) == 0 ? 0xFD8A0000UL : (0xFEC20000UL + ((n) - 1) * 0x10000))

VOID
GpioPinSetDirection (
  IN UINT8 Group,
  IN UINT8 Pin,
  IN GPIO_PIN_DIRECTION Direction
  )
{
    DEBUG((EFI_D_WARN, "GpioPinSetDirection Group:%d Pin:%d\n", Group, Pin));
    MmioWrite32 (GPIO_BASE (Group) + GPIO_SWPORT_DDR (Pin),
                 GPIO_WRITE_MASK (Pin) | GPIO_VALUE_MASK (Pin, Direction));
}

VOID
GpioPinWrite (
  IN UINT8 Group,
  IN UINT8 Pin,
  IN BOOLEAN Value
  )
{
    DEBUG((EFI_D_WARN, "GpioPinWrite Group:%d Pin:%d\n", Group, Pin));
    MmioWrite32 (GPIO_BASE (Group) + GPIO_SWPORT_DR (Pin),
                 GPIO_WRITE_MASK (Pin) | GPIO_VALUE_MASK (Pin, Value));
}

BOOLEAN
GpioPinRead (
  IN UINT8 Group,
  IN UINT8 Pin
  )
{
    CONST UINT32 Value = MmioRead32 (GPIO_BASE (Group) + GPIO_SWPORT_DR (Pin));
    return (Value & GPIO_VALUE_MASK (Pin, 1)) != 0;
}

VOID
GpioPinSetFunction (
  IN UINT8 Group,
  IN UINT8 Pin,
  IN UINT8 Function
  )
{
  ASSERT (Group < GPIO_NGROUPS);

  EFI_PHYSICAL_ADDRESS Reg;
  UINT32 Value = GRF_GPIO_IOMUX_MASK (Pin) | ((UINT32)Function << GRF_GPIO_IOMUX_SHIFT (Pin));

  DEBUG ((DEBUG_INFO, "GPIO: SetFunction %u %02X %04X      0x%lX = 0x%08X\n", Group, Pin, Function, Reg, Value));

  if(Group == 0) {
    if(Pin >= GPIO_PIN_PB4) {
      if(Function >= 8) {
        Reg = PMU2_IOC_BASE - 0xC + GRF_GPIO_IOMUX_REG(Pin);
        Value = GRF_GPIO_IOMUX_MASK (Pin) | ((UINT32)8 << GRF_GPIO_IOMUX_SHIFT (Pin));
        MmioWrite32(Reg, Value);

        Reg = BUS_IOC_BASE + GRF_GPIO_IOMUX_REG(Pin);
        Value = GRF_GPIO_IOMUX_MASK (Pin) | ((UINT32)Function << GRF_GPIO_IOMUX_SHIFT (Pin));
        MmioWrite32(Reg, Value);
      } else {
        Reg = PMU2_IOC_BASE - 0xC + GRF_GPIO_IOMUX_REG(Pin);
        Value = GRF_GPIO_IOMUX_MASK (Pin) | ((UINT32)Function << GRF_GPIO_IOMUX_SHIFT (Pin));
        MmioWrite32(Reg, Value);

        Reg = BUS_IOC_BASE + GRF_GPIO_IOMUX_REG(Pin);
        Value = GRF_GPIO_IOMUX_MASK (Pin);
        MmioWrite32(Reg, Value);
      }
      return;
    } else {
      Reg = PMU1_IOC_BASE + GRF_GPIO_IOMUX_REG(Pin);
    }
  } else {
    Reg = (BUS_IOC_BASE + 0x0020) + ((Group - 1) * 0x8) + GRF_GPIO_IOMUX_REG(Pin);
  }

  MmioWrite32 (Reg, Value);
}

VOID
GpioPinSetPull (
  IN UINT8 Group,
  IN UINT8 Pin,
  IN GPIO_PIN_PULL Pull
  )
{
  // ASSERT (Group < GPIO_NGROUPS);

  // if (Group == 0 && Pin >= GPIO_PIN_PD3 && Pin <= GPIO_PIN_PD6 && Pull == GPIO_PIN_PULL_UP) {
  //   Pull = 3;
  // }

  // CONST EFI_PHYSICAL_ADDRESS Reg = mPinmuxReg[Group].P + GRF_GPIO_P_REG (Pin);
  // CONST UINT32 Value = GRF_GPIO_P_MASK (Pin) | ((UINT32)Pull << GRF_GPIO_P_SHIFT (Pin));

  // DEBUG ((DEBUG_INFO, "GPIO: SetPull     %u %02X %04X      0x%lX = 0x%08X\n", Group, Pin, Pull, Reg, Value));

  // MmioWrite32 (Reg, Value);
}

VOID
GpioPinSetDrive (
  IN UINT8 Group,
  IN UINT8 Pin,
  IN GPIO_PIN_DRIVE Drive
  )
{
  // ASSERT (Group < GPIO_NGROUPS);
  // ASSERT (Drive != GPIO_PIN_DRIVE_DEFAULT);

  // CONST EFI_PHYSICAL_ADDRESS Reg = mPinmuxReg[Group].DS + GRF_GPIO_DS_REG (Pin);
  // CONST UINT32 Value = GRF_GPIO_DS_MASK (Pin) | ((UINT32)Drive << GRF_GPIO_DS_SHIFT (Pin));

  // DEBUG ((DEBUG_INFO, "GPIO: SetDrive    %u %02X %04X      0x%lX = 0x%08X\n", Group, Pin, Drive, Reg, Value));

  // MmioWrite32 (Reg, Value);
}

VOID
GpioPinSetInput (
  IN UINT8 Group,
  IN UINT8 Pin,
  IN GPIO_PIN_INPUT_ENABLE InputEnable
  )
{
  // ASSERT (Group < GPIO_NGROUPS);
  // ASSERT (InputEnable != GPIO_PIN_INPUT_DEFAULT);

  // CONST EFI_PHYSICAL_ADDRESS Reg = mPinmuxReg[Group].IE + GRF_GPIO_IE_REG (Pin);
  // CONST UINT32 Value = GRF_GPIO_IE_MASK (Pin) | ((UINT32)InputEnable << GRF_GPIO_IE_SHIFT (Pin));

  // DEBUG ((DEBUG_INFO, "GPIO: SetInput    %u %02X %04X      0x%lX = 0x%08X\n", Group, Pin, InputEnable, Reg, Value));

  // MmioWrite32 (Reg, Value);
}

VOID
GpioSetIomuxConfig (
  IN CONST GPIO_IOMUX_CONFIG *Configs,
  IN UINT32 NumConfigs
  )
{
  UINT32 Index;

  for (Index = 0; Index < NumConfigs; Index++) {
    CONST GPIO_IOMUX_CONFIG *Mux = &Configs[Index];
    DEBUG ((DEBUG_INFO, "GPIO: IOMUX for pin '%a'\n", Mux->Name));
    GpioPinSetFunction (Mux->Group, Mux->Pin, Mux->Function);
    GpioPinSetPull (Mux->Group, Mux->Pin, Mux->Pull);
    if (Mux->Drive != GPIO_PIN_DRIVE_DEFAULT) {
      GpioPinSetDrive (Mux->Group, Mux->Pin, Mux->Drive);
    }
  }
}
