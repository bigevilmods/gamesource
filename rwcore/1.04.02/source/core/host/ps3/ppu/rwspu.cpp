// (c) Electronic Arts. All Rights Reserved.

/// \file
/// SPU management.

// ***********************************************************************************************************
// Includes
#include "rwsharedheaders.hpp"

#ifdef EA_DEBUG
#include <stdio.h>
#endif
#include "rw/core/spu.h"

// ***********************************************************************************************************
// Defines

namespace
{

// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
// Static Variables


// ***********************************************************************************************************
// Static Functions

} // namespace

namespace rw
{

// ***********************************************************************************************************
// External Functions


// ***********************************************************************************************************
//                                                SPU Definition
// ***********************************************************************************************************

void SPU::Initialize()
{
    rwFUNCTION();

    CHECKRETURN(sys_raw_spu_create(&m_id, 0));

    // Create interrupt handler thread to receive messages from SPU
    CHECKRETURN(sys_ppu_thread_create(&m_thread, InterruptHandler, 0, 100, 0, SYS_PPU_THREAD_CREATE_INTERRUPT, "Interrupt PPU Thread"));
    CHECKRETURN(sys_raw_spu_create_interrupt_tag(m_id, 2, 0, &m_tag));
    CHECKRETURN(sys_interrupt_thread_establish(&m_handle, m_tag, m_thread, reinterpret_cast<rw::HostPtr>(this)));
    CHECKRETURN(sys_raw_spu_set_int_mask(m_id, 2, 7));

    rwRETURNVOID();
}


void SPU::Release()
{
    rwFUNCTION();

    CHECKRETURN(sys_raw_spu_set_int_mask(m_id, 2, 0));
    CHECKRETURN(sys_raw_spu_destroy(m_id));

    rwRETURNVOID();
}

// Execute file on SPU. This will not be supported in the future.
void SPU::Execute(const RwChar *filename)
{
    rwFUNCTION();
    uint32_t entry;
    CHECKRETURN(sys_raw_spu_load(m_id, filename, &entry));
    SetNextProgramCounter(static_cast<uint32_t>(entry));
    asm volatile ("eieio");
    SetRunControl(m_runControlRun);
    rwRETURNVOID();
}


uint32_t SPU::ToString(RwChar *buffer, uint32_t value, uint32_t base, uint32_t flags, uint32_t align)
{
    rwFUNCTION();
    const RwChar *digits = 0 != (m_flagUpper & flags)
        ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" : "0123456789abcdefghijklmnopqrstuvwxyz";

    RwChar *current = buffer;
    if (static_cast<int32_t>(value) < 0 && 0 != (m_flagSigned & flags))
    {
        *current++ = '-';
        value = 0 - value;
    }

    RwChar temp[34];
    uint32_t numDigits = 0;
    while (0 != value)
    {
        temp[numDigits++] = digits[value % base];
        value /= base;
    }
    if (0 == numDigits)
    {
        temp[numDigits++] = '0';
    }

    while (numDigits < align)
    {
        if (0 != (m_flagZero & flags))
        {
            *current++ = '0';
        }
        else
        {
            *current++ = ' ';
        }
        --align;
    }

    while (numDigits > 0)
    {
        *current++ = temp[--numDigits];
    }

    rwRETURN(current - buffer);
}


void SPU::Printf(uint32_t localAddress)
{
    rwFUNCTION();
    // Convert SPU LS address to PU address
    rw::HostPtr puAddress = LocalToEffectiveAddress(localAddress);

    // Print message with SPU<id>: as a prefix
    RwChar buffer[m_bufferSize];
    RwChar *current = buffer;
    RwChar *format
        = reinterpret_cast<RwChar *>(LocalToEffectiveAddress(*reinterpret_cast<uint32_t *>(puAddress)));
    puAddress += 16;
    RwChar c;
    do
    {
        c = *format++;
        if ('%' == c)
        {
            uint32_t align = 0;
            uint32_t type = 0;
        next:
            RwChar f = *format++;
            RwChar *temp;
            switch (f)
            {
            case '0':
                if (0 == align)
                {
                    type = m_flagZero;
                    goto next;
                }
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                align = 10 * align + f - '0';
                goto next;
            case 'c':
                *current++ = *reinterpret_cast<RwChar *>(puAddress);
                puAddress += 16;
                break;
            case 'o':
                current += ToString(current, *reinterpret_cast<uint32_t *>(puAddress), 8, type, align);
                puAddress += 16;
                break;
            case 'x':
                current += ToString(current, *reinterpret_cast<uint32_t *>(puAddress), 16, type, align);
                puAddress += 16;
                break;
            case 'X':
                current += ToString(current, *reinterpret_cast<uint32_t *>(puAddress), 16,
                                    type | m_flagUpper, align);
                puAddress += 16;
                break;
            case 'd':
                current += ToString(current, *reinterpret_cast<uint32_t *>(puAddress), 10,
                                    type | m_flagSigned, align);
                puAddress += 16;
                break;
            case 'f':
                current += sprintf(current, "%f", *reinterpret_cast<float64_t *>(puAddress));
                puAddress += 16;
                break;
            case 's':
                temp = reinterpret_cast<RwChar *>
                    (LocalToEffectiveAddress(*reinterpret_cast<uint32_t *>(puAddress)));
                while (0 != *temp)
                {
                    *current++ = *temp++;
                }
                puAddress += 16;
                break;
            case '%':
                *current++ = '%';
                break;
            case 0:
                *current++ = 0;
                c = 0;
                break;
            }
        }
        else
        {
            *current++ = c;
        }
    } while (0 != c);
    printf("SPU%d: %s", m_id, buffer);

    // Return value to SPU
    PutMail(static_cast<uint32_t>(current - buffer - 1));
    rwRETURNVOID();
}


void SPU::HandleInterrupt()
{
    rwFUNCTION();
    // Get interrupt status to determine whether this interrupt was caused by an SPU message
    uint64_t status;
    CHECKRETURN(sys_raw_spu_get_int_stat(m_id, 2, &status));

    if (m_statusMask == (m_statusMask & status))
    {
        // Read mailbox value
        uint32_t mail = GetMail();

        // Process message
        Printf(mail & 0x00ffffff);

        // Reset interrupt status
        CHECKRETURN(sys_raw_spu_set_int_stat(m_id, 2, m_statusMask & status));
    }
    else
    {
        // Reset interrupt status
        CHECKRETURN(sys_raw_spu_set_int_stat(m_id, 2, status));
    }

    sys_interrupt_thread_eoi();
    rwRETURNVOID();
}

void SPU::InterruptHandler(uint64_t spu)
{
    rwFUNCTION();
    (reinterpret_cast<SPU *>(static_cast<uint32_t>(spu)))->HandleInterrupt();
    rwRETURNVOID();
}

} //namespace rw
