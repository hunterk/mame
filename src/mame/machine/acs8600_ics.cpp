// license:BSD-3-Clause
// copyright-holders:Carl

// TODO: multibus

#include "emu.h"
#include "machine/acs8600_ics.h"
#include "cpu/z80/z80.h"
#include "machine/z80sio.h"
#include "machine/am9513.h"
#include "bus/rs232/rs232.h"

DEFINE_DEVICE_TYPE(ACS8600_ICS, acs8600_ics_device, "acs8600_ics", "Altos ACS8600 Intelligent Serial Concentrator")

acs8600_ics_device::acs8600_ics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ACS8600_ICS, tag, owner, clock),
	m_icscpu(*this, "icscpu"),
	m_out_irq1_func(*this),
	m_out_irq2_func(*this),
	m_host_space_device(*this, finder_base::DUMMY_TAG),
	m_host_space_index(-1)
{
}

ROM_START(acs8600_ics)
	ROM_REGION(0x1000, "icscpu", 0)
	ROM_LOAD("162-13914-001.bin", 0x0000, 0x1000, CRC(ab41768a) SHA1(aee04574dea4c17112431536d3b7e3984b8afdc2))
ROM_END

const tiny_rom_entry *acs8600_ics_device::device_rom_region() const
{
	return ROM_NAME(acs8600_ics);
}

WRITE8_MEMBER(acs8600_ics_device::hiaddr_w)
{
	m_hiaddr = data;
}

WRITE8_MEMBER(acs8600_ics_device::ctrl_w)
{
	m_ctrl = data;
	m_out_irq1_func(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(acs8600_ics_device::hostram_r)
{
	return m_maincpu_mem->read_byte((m_hiaddr << 16) | (BIT(m_ctrl, 0) << 15) | (offset & 0x7fff));
}

WRITE8_MEMBER(acs8600_ics_device::hostram_w)
{
	m_maincpu_mem->write_byte((m_hiaddr << 16) | (BIT(m_ctrl, 0) << 15) | (offset & 0x7fff), data);
}

WRITE_LINE_MEMBER(acs8600_ics_device::attn_w)
{
	m_icscpu->set_input_line(INPUT_LINE_NMI, state);
}

static DEVICE_INPUT_DEFAULTS_START(altos8600_terminal)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void acs8600_ics_device::ics_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("icscpu", 0);
	map(0x1000, 0x17ff).ram();
	map(0x8000, 0xffff).rw(FUNC(acs8600_ics_device::hostram_r), FUNC(acs8600_ics_device::hostram_w));
}

void acs8600_ics_device::ics_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x04, 0x07).rw("sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x08, 0x0b).rw("sio3", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x0c, 0x0f).rw("sio4", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x10, 0x11).rw("stc1", FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0x14, 0x15).rw("stc2", FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0x18, 0x18).w(FUNC(acs8600_ics_device::ctrl_w));
	map(0x1c, 0x1c).w(FUNC(acs8600_ics_device::hiaddr_w));
}

static const z80_daisy_config ics_daisy_chain[] =
{
	"sio1",
	"sio2",
	"sio3",
	"sio4",
	nullptr
};

MACHINE_CONFIG_START(acs8600_ics_device::device_add_mconfig)
	MCFG_DEVICE_ADD(m_icscpu, Z80, 4_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(ics_mem)
	MCFG_DEVICE_IO_MAP(ics_io)
	MCFG_Z80_DAISY_CHAIN(ics_daisy_chain)

	am9513_device &stc1(AM9513(config, "stc1", 1.8432_MHz_XTAL));
	stc1.out1_cb().set("sio1", FUNC(z80sio_device::rxca_w));
	stc1.out1_cb().append("sio1", FUNC(z80sio_device::txca_w));
	stc1.out2_cb().set("sio1", FUNC(z80sio_device::rxtxcb_w));
	stc1.out3_cb().set("sio2", FUNC(z80sio_device::rxca_w));
	stc1.out3_cb().append("sio2", FUNC(z80sio_device::txca_w));
	stc1.out4_cb().set("sio2", FUNC(z80sio_device::rxtxcb_w));
	stc1.out5_cb().set("sio3", FUNC(z80sio_device::rxca_w));
	stc1.out5_cb().append("sio3", FUNC(z80sio_device::txca_w));

	am9513_device &stc2(AM9513(config, "stc2", 1.8432_MHz_XTAL));
	stc2.out1_cb().set("sio3", FUNC(z80sio_device::rxtxcb_w));
	stc2.out2_cb().set("sio4", FUNC(z80sio_device::rxca_w));
	stc2.out2_cb().append("sio4", FUNC(z80sio_device::txca_w));
	stc2.out3_cb().set("sio4", FUNC(z80sio_device::rxtxcb_w));

	z80sio_device &sio1(Z80SIO(config, "sio1", 8_MHz_XTAL/2));
	sio1.out_txda_callback().set("rs2321a", FUNC(rs232_port_device::write_txd));
	sio1.out_dtra_callback().set("rs2321a", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsa_callback().set("rs2321a", FUNC(rs232_port_device::write_rts));
	sio1.out_txdb_callback().set("rs2321b", FUNC(rs232_port_device::write_txd));
	sio1.out_dtrb_callback().set("rs2321b", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsb_callback().set("rs2321b", FUNC(rs232_port_device::write_rts));
	sio1.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio1.set_cputag(m_icscpu);

	MCFG_DEVICE_ADD("rs2321a", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio1", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio1", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio1", z80sio_device, ctsa_w))
	MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", altos8600_terminal)

	MCFG_DEVICE_ADD("rs2321b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio1", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio1", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio1", z80sio_device, ctsb_w))

	z80sio_device &sio2(Z80SIO(config, "sio2", 8_MHz_XTAL/2));
	sio2.out_txda_callback().set("rs2322a", FUNC(rs232_port_device::write_txd));
	sio2.out_dtra_callback().set("rs2322a", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsa_callback().set("rs2322a", FUNC(rs232_port_device::write_rts));
	sio2.out_txdb_callback().set("rs2322b", FUNC(rs232_port_device::write_txd));
	sio2.out_dtrb_callback().set("rs2322b", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsb_callback().set("rs2322b", FUNC(rs232_port_device::write_rts));
	sio2.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio2.set_cputag(m_icscpu);

	MCFG_DEVICE_ADD("rs2322a", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio2", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio2", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio2", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("rs2322b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio2", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio2", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio2", z80sio_device, ctsb_w))

	z80sio_device &sio3(Z80SIO(config, "sio3", 8_MHz_XTAL/2));
	sio3.out_txda_callback().set("rs2323a", FUNC(rs232_port_device::write_txd));
	sio3.out_dtra_callback().set("rs2323a", FUNC(rs232_port_device::write_dtr));
	sio3.out_rtsa_callback().set("rs2323a", FUNC(rs232_port_device::write_rts));
	sio3.out_txdb_callback().set("rs2323b", FUNC(rs232_port_device::write_txd));
	sio3.out_dtrb_callback().set("rs2323b", FUNC(rs232_port_device::write_dtr));
	sio3.out_rtsb_callback().set("rs2323b", FUNC(rs232_port_device::write_rts));
	sio3.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio3.set_cputag(m_icscpu);

	MCFG_DEVICE_ADD("rs2323a", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio3", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio3", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio3", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("rs2323b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio3", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio3", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio3", z80sio_device, ctsb_w))

	z80sio_device &sio4(Z80SIO(config, "sio4", 8_MHz_XTAL/2));
	sio4.out_txda_callback().set("rs2324a", FUNC(rs232_port_device::write_txd));
	sio4.out_dtra_callback().set("rs2324a", FUNC(rs232_port_device::write_dtr));
	sio4.out_rtsa_callback().set("rs2324a", FUNC(rs232_port_device::write_rts));
	sio4.out_txdb_callback().set("rs2324b", FUNC(rs232_port_device::write_txd));
	sio4.out_dtrb_callback().set("rs2324b", FUNC(rs232_port_device::write_dtr));
	sio4.out_rtsb_callback().set("rs2324b", FUNC(rs232_port_device::write_rts));
	sio4.out_int_callback().set_inputline(m_icscpu, INPUT_LINE_IRQ0);
	sio4.set_cputag(m_icscpu);

	MCFG_DEVICE_ADD("rs2324a", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio4", z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio4", z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio4", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("rs2324b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio4", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio4", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio4", z80sio_device, ctsb_w))
MACHINE_CONFIG_END

void acs8600_ics_device::device_resolve_objects()
{
	m_maincpu_mem = &m_host_space_device->space(m_host_space_index);
	m_out_irq1_func.resolve_safe();
	m_out_irq2_func.resolve_safe();
}

void acs8600_ics_device::device_start()
{
}

