// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria, AJR
/**********************************************************************

    Dynax blitter, "revision 2" (TC17G032AP-0246 custom DIP64)

**********************************************************************/
#ifndef MAME_VIDEO_DYNAX_BLITTER_REV2_H
#define MAME_VIDEO_DYNAX_BLITTER_REV2_H

#pragma once


//**************************************************************************
//  CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DYNAX_BLITTER_REV2_VRAM_OUT_CB(_devcb) \
	downcast<dynax_blitter_rev2_device &>(*device).set_vram_out_cb(DEVCB_##_devcb);
#define MCFG_DYNAX_BLITTER_REV2_SCROLLX_CB(_devcb) \
	downcast<dynax_blitter_rev2_device &>(*device).set_scrollx_cb(DEVCB_##_devcb);
#define MCFG_DYNAX_BLITTER_REV2_SCROLLY_CB(_devcb) \
	downcast<dynax_blitter_rev2_device &>(*device).set_scrolly_cb(DEVCB_##_devcb);
#define MCFG_DYNAX_BLITTER_REV2_READY_CB(_devcb) \
	downcast<dynax_blitter_rev2_device &>(*device).set_ready_cb(DEVCB_##_devcb);

#define MCFG_CDRACULA_BLITTER_VRAM_OUT_CB(_devcb) \
	downcast<cdracula_blitter_device &>(*device).set_vram_out_cb(DEVCB_##_devcb);
#define MCFG_CDRACULA_BLITTER_SCROLLX_CB(_devcb) \
	downcast<cdracula_blitter_device &>(*device).set_scrollx_cb(DEVCB_##_devcb);
#define MCFG_CDRACULA_BLITTER_SCROLLY_CB(_devcb) \
	downcast<cdracula_blitter_device &>(*device).set_scrolly_cb(DEVCB_##_devcb);
#define MCFG_CDRACULA_BLITTER_READY_CB(_devcb) \
	downcast<cdracula_blitter_device &>(*device).set_ready_cb(DEVCB_##_devcb);
#define MCFG_CDRACULA_BLITTER_DEST_CB(_devcb) \
	downcast<cdracula_blitter_device &>(*device).set_blit_dest_cb(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dynax_blitter_rev2_device

class dynax_blitter_rev2_device : public device_t, public device_rom_interface
{
public:
	// construction/destruction
	dynax_blitter_rev2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// static configuration
	template<class Object> devcb_base &set_vram_out_cb(Object &&cb) { return m_vram_out_cb.set_callback(std::forward<Object>(cb)); }
	template<class Object> devcb_base &set_scrollx_cb(Object &&cb) { return m_scrollx_cb.set_callback(std::forward<Object>(cb)); }
	template<class Object> devcb_base &set_scrolly_cb(Object &&cb) { return m_scrolly_cb.set_callback(std::forward<Object>(cb)); }
	template<class Object> devcb_base &set_ready_cb(Object &&cb) { return m_ready_cb.set_callback(std::forward<Object>(cb)); }
	auto vram_out_cb() { return m_vram_out_cb.bind(); }
	auto scrollx_cb() { return m_scrollx_cb.bind(); }
	auto scrolly_cb() { return m_scrolly_cb.bind(); }
	auto ready_cb() { return m_ready_cb.bind(); }

	// write handlers
	DECLARE_WRITE8_MEMBER(pen_w);
	virtual DECLARE_WRITE8_MEMBER(regs_w);

	// getter
	u8 blit_pen() const { return m_blit_pen; }

protected:
	// delegated construction
	dynax_blitter_rev2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override { }

	// internal helpers
	void plot_pixel(int x, int y, int pen);
	u32 blitter_draw(u32 src, int pen, int x, int y);
	void blitter_start();
	void scroll_w(u8 data);

	// device callbacks
	devcb_write8 m_vram_out_cb;
	devcb_write8 m_scrollx_cb;
	devcb_write8 m_scrolly_cb;
	devcb_write_line m_ready_cb;

	// internal registers
	u8 m_blit_pen;
	u8 m_blit_wrap_enable;
	u8 m_blit_x;
	u8 m_blit_y;
	u8 m_blit_flags;
	u32 m_blit_src;
};

// ======================> cdracula_blitter_device

class cdracula_blitter_device : public dynax_blitter_rev2_device
{
public:
	// construction/destruction
	cdracula_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template<class Object> devcb_base &set_blit_dest_cb(Object &&cb) { return m_blit_dest_cb.set_callback(std::forward<Object>(cb)); }

	// write handlers
	DECLARE_WRITE8_MEMBER(flags_w);
	virtual DECLARE_WRITE8_MEMBER(regs_w) override;

private:
	// device-level overrides
	virtual void device_resolve_objects() override;

	// device callbacks
	devcb_write8 m_blit_dest_cb;
};

// device type declarations
DECLARE_DEVICE_TYPE(DYNAX_BLITTER_REV2, dynax_blitter_rev2_device)
DECLARE_DEVICE_TYPE(CDRACULA_BLITTER, cdracula_blitter_device)

#endif // MAME_VIDEO_DYNAX_BLITTER_REV2_H
