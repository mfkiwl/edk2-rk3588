/** @file
*  PCI Host Bridge Library instance for Rockchip platforms
*
*  Copyright (c) 2022, Rockhip Inc. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/Pcie.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/PciRootBridgeIo.h>

GLOBAL_REMOVE_IF_UNREFERENCED
STATIC CHAR16 CONST * CONST mPciHostBridgeLibAcpiAddressSpaceTypeStr[] = {
  L"Mem", L"I/O", L"Bus"
};

#pragma pack (1)
typedef struct {
  ACPI_HID_DEVICE_PATH     AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;
#pragma pack ()

STATIC EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath[] = {
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)sizeof (ACPI_HID_DEVICE_PATH),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCIe
      PCIE_SEGMENT_PCIE30X4
    },
    {
      END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      {
        END_DEVICE_PATH_LENGTH,
        0
      }
    }
  },
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)sizeof (ACPI_HID_DEVICE_PATH),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCIe
      PCIE_SEGMENT_PCIE30X2
    },
    {
      END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      {
        END_DEVICE_PATH_LENGTH,
        0
      }
    }
  },
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)sizeof (ACPI_HID_DEVICE_PATH),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCIe
      PCIE_SEGMENT_PCIE20L0
    },
    {
      END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      {
        END_DEVICE_PATH_LENGTH,
        0
      }
    }
  },
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)sizeof (ACPI_HID_DEVICE_PATH),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCIe
      PCIE_SEGMENT_PCIE20L1
    },
    {
      END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      {
        END_DEVICE_PATH_LENGTH,
        0
      }
    }
  },
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)sizeof (ACPI_HID_DEVICE_PATH),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCIe
      PCIE_SEGMENT_PCIE20L2
    },
    {
      END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      {
        END_DEVICE_PATH_LENGTH,
        0
      }
    }
  },
};

// STATIC PCI_ROOT_BRIDGE mPciRootBridge[] = {
//   {
//     0,                                              // Segment
//     0,                                              // Supports
//     0,                                              // Attributes
//     TRUE,                                           // DmaAbove4G
//     FALSE,                                          // NoExtendedConfigSpace
//     FALSE,                                          // ResourceAssigned
//     EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM |          // AllocationAttributes
//     EFI_PCI_HOST_BRIDGE_MEM64_DECODE,
//     {
//       // Bus
//       0x0,
//       0xf
//     }, {
//       // Io disable
//       0x0,//FixedPcdGet32 (PcdPcieRootPort3x4IoBaseAddress),
//       0x10000 - 1,//FixedPcdGet32 (PcdPcieRootPort3x4IoBaseAddress) + FixedPcdGet32 (PcdPcieRootPort3x4IoSize) - 1,
//       MAX_UINT64 - 0xffff0000 + 1
//     }, {
//       // Mem
//       FixedPcdGet32 (PcdPcieRootPort3x4MemBaseAddress),
//       FixedPcdGet32 (PcdPcieRootPort3x4MemBaseAddress) + FixedPcdGet32 (PcdPcieRootPort3x4MemSize) - 1
//     }, {
//       // MemAbove4G
// 	 FixedPcdGet64 (PcdPcieRootPort3x4MemBaseAddress64),
// 	 FixedPcdGet64 (PcdPcieRootPort3x4MemBaseAddress64) + FixedPcdGet64 (PcdPcieRootPort3x4MemSize64) - 1

//     }, {
//       // PMem
//       MAX_UINT64,
//       0
//     }, {
//       // PMemAbove4G
//       MAX_UINT64,
//       0
//     },
//     (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath[0]
//   }
// };

PCI_ROOT_BRIDGE mPciRootBridges[NUM_PCIE_CONTROLLER];

BOOLEAN
IsPcieNumEnabled(
  UINTN PcieNum
  )
{
  BOOLEAN Enabled = FALSE;
  switch (PcieNum)
  {
	/* No bifurcation config yet */
	case PCIE_SEGMENT_PCIE30X4:
		Enabled = TRUE;
		break;

	case PCIE_SEGMENT_PCIE30X2:
		Enabled = FALSE;
		break;

	case PCIE_SEGMENT_PCIE20L0:
		Enabled = (PcdGet32(PcdComboPhy0Mode) == 1); // COMBO_PHY_MODE_PCIE
		break;

	case PCIE_SEGMENT_PCIE20L1:
		Enabled = (PcdGet32(PcdComboPhy1Mode) == 1); // COMBO_PHY_MODE_PCIE
		break;

	case PCIE_SEGMENT_PCIE20L2:
		Enabled = (PcdGet32(PcdComboPhy2Mode) == 1); // COMBO_PHY_MODE_PCIE
		break;

	default:
	break;
  }

  if(FixedPcdGetBool(PcdSocIs3588S) == TRUE && (PcieNum == PCIE_SEGMENT_PCIE30X4 || PcieNum == PCIE_SEGMENT_PCIE30X2))
    Enabled = FALSE;
  return Enabled;
}

/**
  Return all the root bridge instances in an array.

  @param Count  Return the count of root bridge instances.

  @return All the root bridge instances in an array.
          The array should be passed into PciHostBridgeFreeRootBridges()
          when it's not used.
**/
PCI_ROOT_BRIDGE *
EFIAPI
PciHostBridgeGetRootBridges (
  UINTN                             *Count
  )
{
  DEBUG((DEBUG_ERROR, "PciHostBridgeGetRootBridges\n"));
  UINTN         Idx;
  UINTN         Loop;
  for(Idx = 0, Loop = 0; Idx < NUM_PCIE_CONTROLLER; Idx++) {
    if(IsPcieNumEnabled(Idx) == FALSE)
      continue;

    mPciRootBridges[Loop].Segment               = Idx;
    mPciRootBridges[Loop].Supports              = 0;
    mPciRootBridges[Loop].Attributes            = 0;
    mPciRootBridges[Loop].DmaAbove4G            = TRUE;
    mPciRootBridges[Loop].NoExtendedConfigSpace = FALSE;
    mPciRootBridges[Loop].ResourceAssigned      = FALSE;
    mPciRootBridges[Loop].AllocationAttributes  = EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM|EFI_PCI_HOST_BRIDGE_MEM64_DECODE;

    mPciRootBridges[Loop].Bus.Base              = 0x0;
    mPciRootBridges[Loop].Bus.Limit             = 0xf;

    mPciRootBridges[Loop].Io.Base               = 0x0;
    mPciRootBridges[Loop].Io.Limit              = 0x10000 - 1;
    mPciRootBridges[Loop].Io.Translation        = MAX_UINT64 -
                                                  0xffff*Idx + 1;

    mPciRootBridges[Loop].Mem.Base              = PCIE_SEG0_CFG_BASE + Idx*PCIE_CFG_BASE_DIFF + PCIE_MEM_OFFSET;
    mPciRootBridges[Loop].Mem.Limit             = mPciRootBridges[Loop].Mem.Base + PCIE_MEM_SIZE - 1;

    mPciRootBridges[Loop].MemAbove4G.Base       = PCIE_SEG0_MEM64_BASE + Idx*PCIE_MEM64_SIZE + 0x1000000;
    mPciRootBridges[Loop].MemAbove4G.Limit      = PCIE_SEG0_MEM64_BASE + Idx*PCIE_MEM64_SIZE - 1;

    mPciRootBridges[Loop].PMem.Base             = MAX_UINT64;
    mPciRootBridges[Loop].PMem.Limit            = 0;
    mPciRootBridges[Loop].PMemAbove4G.Base      = MAX_UINT64;
    mPciRootBridges[Loop].PMemAbove4G.Limit     = 0;
    mPciRootBridges[Loop].DevicePath            = (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath[Idx];
    
    DEBUG((DEBUG_ERROR, "0x%llx 0x%llx\n", mPciRootBridges[Loop].Mem.Base, mPciRootBridges[Loop].MemAbove4G.Base));
    Loop++;
  }
  
  *Count = Loop;
  if(Loop == 0) return NULL;

  return mPciRootBridges;
}

/**
  Free the root bridge instances array returned from PciHostBridgeGetRootBridges().

  @param Bridges The root bridge instances array.
  @param Count   The count of the array.
**/
VOID
EFIAPI
PciHostBridgeFreeRootBridges (
  PCI_ROOT_BRIDGE                   *Bridges,
  UINTN                             Count
  )
{
}

/**
  Inform the platform that the resource conflict happens.

  @param HostBridgeHandle Handle of the Host Bridge.
  @param Configuration    Pointer to PCI I/O and PCI memory resource
                          descriptors. The Configuration contains the resources
                          for all the root bridges. The resource for each root
                          bridge is terminated with END descriptor and an
                          additional END is appended indicating the end of the
                          entire resources. The resource descriptor field
                          values follow the description in
                          EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL.\
                          SubmitResources().
**/
VOID
EFIAPI
PciHostBridgeResourceConflict (
  EFI_HANDLE                        HostBridgeHandle,
  VOID                              *Configuration
  )
{
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptor;
  UINTN                             RootBridgeIndex;

  DEBUG ((DEBUG_ERROR, "PciHostBridge: Resource conflict happens!\n"));

  RootBridgeIndex = 0;
  Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Configuration;

  while (Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {

    DEBUG ((DEBUG_ERROR, "RootBridge[%d]:\n", RootBridgeIndex++));

    for (; Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR; Descriptor++) {
      ASSERT (Descriptor->ResType <
              ARRAY_SIZE (mPciHostBridgeLibAcpiAddressSpaceTypeStr));

      DEBUG ((DEBUG_ERROR, " %s: Length/Alignment = 0x%lx / 0x%lx\n",
              mPciHostBridgeLibAcpiAddressSpaceTypeStr[Descriptor->ResType],
              Descriptor->AddrLen, Descriptor->AddrRangeMax
              ));

      if (Descriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) {
        DEBUG ((DEBUG_ERROR, "     Granularity/SpecificFlag = %ld / %02x%s\n",
                Descriptor->AddrSpaceGranularity, Descriptor->SpecificFlag,
                ((Descriptor->SpecificFlag &
                  EFI_ACPI_MEMORY_RESOURCE_SPECIFIC_FLAG_CACHEABLE_PREFETCHABLE
                  ) != 0) ? L" (Prefetchable)" : L""
                ));
      }
    }
    //
    // Skip the END descriptor for root bridge
    //
    ASSERT (Descriptor->Desc == ACPI_END_TAG_DESCRIPTOR);
    Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)(
                   (EFI_ACPI_END_TAG_DESCRIPTOR *)Descriptor + 1
                   );
  }
}
