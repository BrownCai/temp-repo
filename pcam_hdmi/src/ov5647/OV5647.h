/*
 * OV5647.h
 *
 *  Created on: May 26, 2016
 *      Author: Elod
 */

#ifndef OV5647_H_
#define OV5647_H_

#include <sstream>
#include <iostream>
#include <cstdio>
#include <climits>

#include "../hdmi/VideoOutput.h"
#include "../ov5647/GPIO_Client.h"
#include "../ov5647/I2C_Client.h"

#define SIZEOF_ARRAY(x) sizeof(x)/sizeof(x[0])
#define MAP_ENUM_TO_CFG(en, cfg) en, cfg, SIZEOF_ARRAY(cfg)

namespace digilent {

typedef enum {OK=0, ERR_LOGICAL, ERR_GENERAL} Errc;

namespace OV5647_cfg {
	using config_word_t = struct { uint16_t addr; uint8_t data; } ;
	using mode_t = enum { MODE_720P_1280_720_60fps = 0, MODE_1080P_1920_1080_15fps,
		MODE_1080P_1920_1080_30fps, MODE_1080P_1920_1080_30fps_336M_MIPI,
		MODE_1080P_1920_1080_30fps_336M_1LANE_MIPI, MODE_END } ;
	using config_modes_t = struct { mode_t mode; config_word_t const* cfg; size_t cfg_size; };
	using test_t = enum { TEST_DISABLED = 0, TEST_EIGHT_COLOR_BAR, TEST_END };
	using awb_t = enum { AWB_DISABLED = 0, AWB_SIMPLE, AWB_ADVANCED, AWB_END };
	using config_awb_t = struct { awb_t awb; config_word_t const* cfg; size_t cfg_size; };
	using isp_format_t = enum { ISP_RAW = 0, ISP_RGB, ISP_END };
	uint16_t const OV5647_REG_PRE_ISP_TEST_SET1 = 0x503D;
	uint16_t const OV5647_FORMAT_MUX_CONTROL = 0x501f;

	config_word_t const cfg_advanced_awb_[] =
	{
		{0x5001 ,0x01}
	};

	config_word_t const cfg_simple_awb_[] =
	{
		// Disable Advanced AWB
		{0x518d ,0x00},
		{0x518f ,0x20},
		{0x518e ,0x00},
		{0x5190 ,0x20},
		{0x518b ,0x00},
		{0x518c ,0x00},
		{0x5187 ,0x10},
		{0x5188 ,0x10},
		{0x5189 ,0x40},
		{0x518a ,0x40},
		{0x5186 ,0x10},
		{0x5181 ,0x58},
		{0x5184 ,0x25},
		{0x5182 ,0x11},

		// Enable simple AWB
		{0x3406 ,0x00},
		{0x5183 ,0x80},
		{0x5191 ,0xff},
		{0x5192 ,0x00},
		{0x5001 ,0x03}
	};

	config_word_t const cfg_disable_awb_[] =
	{
		{0x5001 ,0x02}
	};

	config_word_t const cfg_720p_60fps_[] =
	{//1280 x 720 binned, RAW10, MIPISCLK=280M, SCLK=56Mz, PCLK=56M

			{0x0100, 0x00},// ; software standby
			{0x3035, 0x21},// ; PLL
			{0x3036, 0x46},// ; PLL
			{0x303c, 0x11},// ; PLL
			{0x3821, 0x07},// ; ISP mirror on, Sensor mirror on, bin on
			{0x3820, 0x41},// ; ISP flip off, Sensor flip off, bin on
			{0x3612, 0x59},// ; analog control
			{0x3618, 0x00},// ; analog control
			{0x380c, 0x07},// ; HTS = 1896
			{0x380d, 0x68},// ; HTS
			{0x380e, 0x03},// ; VTS = 984
			{0x380f, 0xd8},// ; VTS
			{0x3814, 0x31},// ; X INC
			{0x3815, 0x31},// ; Y INC
			{0x3708, 0x64},// ; analog control
			{0x3709, 0x52},// ; analog control
			{0x3808, 0x05},// ; X OUTPUT SIZE = 1280
			{0x3809, 0x00},// ; X OUTPUT SIZE
			{0x380a, 0x02},// ; Y OUTPUT SIZE 3c0: 960
			{0x380b, 0xd0},// ; Y OUTPUT SIZE 2d0: 720
			{0x3800, 0x00},// ; X Start
			{0x3801, 0x18},// ; X Start
			{0x3802, 0x00},// ; Y Start
			{0x3803, 0x0e},// ; Y Start
			{0x3804, 0x0a},// ; X End
			{0x3805, 0x27},// ; X End
			{0x3806, 0x07},// ; Y End
			{0x3807, 0x95},// ; Y End
			// banding filter
			{0x3a08, 0x01},// ; B50
			{0x3a09, 0x27},// ; B50
			{0x3a0a, 0x00},// ; B60
			{0x3a0b, 0xf6},// ; B60
			{0x3a0d, 0x04},// ; B50 max
			{0x3a0e, 0x03},// ; B60 max
			{0x4004, 0x02},// ; black line number
			{0x4837, 0x24},// ; MIPI pclk period
			{0x3034, 0x1a},
			{0x370c, 0x03},
			{0x4300, 0x00},
			{0x501f, 0x03},
			{0x4009, 0x00},
			{0x0100, 0x01},// ; wake up from software standby
	};
	config_word_t const cfg_1080p_15fps_[] =
	{//1920 x 1080 @ 15 fps, RAW10, MIPISCLK=210, SCLK=42MHz, PCLK=42M
		// PLL1 configuration
		// [7:4]=0100 System clock divider /4, [3:0]=0001 Scale divider for MIPI /1
		{0x3035, 0x41},
		// [7:0]=105 PLL multiplier
		{0x3036, 0x69},
		// [4]=0 PLL root divider /1, [3:0]=5 PLL pre-divider /1.5
		{0x3037, 0x05},
		// [5:4]=01 PCLK root divider /2, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
		{0x3108, 0x11},

		// [6:4]=001 PLL charge pump, [3:0]=1010 MIPI 10-bit mode
		{0x3034, 0x1A},

		// [3:0]=0 X address start high byte
		{0x3800, (336 >> 8) & 0x0F},
		// [7:0]=0 X address start low byte
		{0x3801, 336 & 0xFF},
		// [2:0]=0 Y address start high byte
		{0x3802, (426 >> 8) & 0x07},
		// [7:0]=0 Y address start low byte
		{0x3803, 426 & 0xFF},

		// [3:0] X address end high byte
		{0x3804, (2287 >> 8) & 0x0F},
		// [7:0] X address end low byte
		{0x3805, 2287 & 0xFF},
		// [2:0] Y address end high byte
		{0x3806, (1529 >> 8) & 0x07},
		// [7:0] Y address end low byte
		{0x3807, 1529 & 0xFF},

		// [3:0]=0 timing hoffset high byte
		{0x3810, (16 >> 8) & 0x0F},
		// [7:0]=0 timing hoffset low byte
		{0x3811, 16 & 0xFF},
		// [2:0]=0 timing voffset high byte
		{0x3812, (12 >> 8) & 0x07},
		// [7:0]=0 timing voffset low byte
		{0x3813, 12 & 0xFF},

		// [3:0] Output horizontal width high byte
		{0x3808, (1920 >> 8) & 0x0F},
		// [7:0] Output horizontal width low byte
		{0x3809, 1920 & 0xFF},
		// [2:0] Output vertical height high byte
		{0x380a, (1080 >> 8) & 0x7F},
		// [7:0] Output vertical height low byte
		{0x380b, 1080 & 0xFF},

		// HTS line exposure time in # of pixels Tline=HTS/sclk
		{0x380c, (2500 >> 8) & 0x1F},
		{0x380d, 2500 & 0xFF},
		// VTS frame exposure time in # lines
		{0x380e, (1120 >> 8) & 0xFF},
		{0x380f, 1120 & 0xFF},

		// [7:4]=0x1 horizontal odd subsample increment, [3:0]=0x1 horizontal even subsample increment
		{0x3814, 0x11},
		// [7:4]=0x1 vertical odd subsample increment, [3:0]=0x1 vertical even subsample increment
		{0x3815, 0x11},

		// [2]=0 ISP mirror, [1]=0 sensor mirror, [0]=0 no horizontal binning
		{0x3821, 0x07},

		// little MIPI shit: global timing unit, period of PCLK in ns * 2(depends on # of lanes)
		{0x4837, 48}, // 1/42M*2

		// Undocumented anti-green settings
		{0x3618, 0x00}, // Removes vertical lines appearing under bright light
		{0x3612, 0x59},
		{0x3708, 0x64},
		{0x3709, 0x52},
		{0x370c, 0x03},

		// [7:4]=0x0 Formatter RAW, [3:0]=0x0 BGBG/GRGR
		{0x4300, 0x00},
		// [2:0]=0x3 Format select ISP RAW (DPC)
		{0x501f, 0x03}
	};
	config_word_t const cfg_1080p_30fps_[] =
	{//1920 x 1080 @ 30fps, RAW10, MIPISCLK=420, SCLK=84MHz, PCLK=84M
			// TO DO: register configurations
	};
	config_word_t const cfg_1080p_30fps_336M_mipi_[] =
	{//1920 x 1080 @ 30fps, RAW10, MIPISCLK=672, SCLK=67.2MHz, PCLK=134.4M
			// TO DO: register configurations
	};
	config_word_t const cfg_1080p_30fps_336M_1lane_mipi_[] =
	{//1920 x 1080 @ 30fps, RAW10, MIPISCLK=672, SCLK=67.2MHz, PCLK=134.4M
			// TO DO: register configurations
	};
	config_word_t const cfg_init_[] =
	{
		  {0x4800, 0x24},
		  {0x0100, 0x00},
		  {0x0103, 0x01},
		  {0x3035, 0x11},
		  {0x3036, 0x64},
		  {0x303c, 0x11},
		  {0x3821, 0x07},
		  {0x3820, 0x41},
		  {0x370c, 0x0f},
		  {0x3612, 0x59},
		  {0x3618, 0x00},
		  {0x5000, 0x06},

//		  {0x5000, 0x06},
		  {0x5001, 0x00},
		  {0x5002, 0x41},

		  {0x5003, 0x08},
		  {0x5a00, 0x08},
		  {0x3000, 0xff},
		  {0x3001, 0xff},
		  {0x3002, 0xff},
		  {0x301d, 0xf0},
		  {0x3a18, 0x00},
		  {0x3a19, 0xf8},
		  {0x3c01, 0x80},
		  {0x3b07, 0x0c},
		  {0x3500, 0x00},
		  {0x3501, 0x29},
		  {0x3502, 0xe0},
		  {0x380c, 0x07},
		  {0x380d, 0x00},
		  {0x380e, 0x02}, // edited
		  {0x380f, 0xee}, // edited
//		  {0x380e, 0x05}, // original
//		  {0x380f, 0xd0}, // original
		  {0x3814, 0x31},
		  {0x3815, 0x31},
		  {0x3708, 0x64},
		  {0x3709, 0x52},
		  {0x3808, 0x05},
		  {0x3809, 0x00},
		  {0x380a, 0x02},
		  {0x380b, 0xd0},
		  {0x3800, 0x00},
		  {0x3801, 0x18},
		  {0x3802, 0x00},
		  {0x3803, 0xf8},
		  {0x3804, 0x0a},
		  {0x3805, 0x27},
		  {0x3806, 0x06},
		  {0x3807, 0xa7},
		  {0x3630, 0x2e},
		  {0x3632, 0xe2},
		  {0x3633, 0x23},
		  {0x3634, 0x44},
		  {0x3620, 0x64},
		  {0x3621, 0xe0},
		  {0x3600, 0x37},
		  {0x3704, 0xa0},
		  {0x3703, 0x5a},
		  {0x3715, 0x78},
		  {0x3717, 0x01},
		  {0x3731, 0x02},
		  {0x370b, 0x60},
		  {0x3705, 0x1a},
		  {0x3f05, 0x02},
		  {0x3f06, 0x10},
		  {0x3f01, 0x0a},
		  {0x3503, 0x00},
		  {0x3a08, 0x01},
		  {0x3a09, 0xbe},
		  {0x3a0a, 0x01},
		  {0x3a0b, 0x74},
		  {0x3a0d, 0x02},
		  {0x3a0e, 0x01},
		  {0x3a0f, 0x58},
		  {0x3a10, 0x50},
		  {0x3a1b, 0x58},
		  {0x3a1e, 0x50},
		  {0x3a11, 0x60},
		  {0x3a1f, 0x28},
		  {0x4001, 0x02},
		  {0x4004, 0x02},
		  {0x4000, 0x09},
		  {0x4050, 0x6e},
		  {0x4051, 0x8f},
		  {0x0100, 0x01},
		  {0x3000, 0x00},
		  {0x3001, 0x00},
		  {0x3002, 0x00},
		  {0x3017, 0xe0},
		  {0x301c, 0xfc},
		  {0x3636, 0x06},
		  {0x3016, 0x08},
		  {0x3827, 0xec},
		  {0x4800, 0x24},
		  {0x3018, 0x44},
		  {0x3035, 0x21},
		  {0x3106, 0xf5},
		  {0x3034, 0x1a},
		  {0x301c, 0xf8},
//		  {0x0100, 0x01}
	};
	config_modes_t const modes[] =
	{
			{ MAP_ENUM_TO_CFG(MODE_720P_1280_720_60fps, cfg_720p_60fps_) },
			{ MAP_ENUM_TO_CFG(MODE_1080P_1920_1080_15fps, cfg_1080p_15fps_) },
			{ MAP_ENUM_TO_CFG(MODE_1080P_1920_1080_30fps, cfg_1080p_30fps_), },
			{ MAP_ENUM_TO_CFG(MODE_1080P_1920_1080_30fps_336M_MIPI, cfg_1080p_30fps_336M_mipi_) },
			{ MAP_ENUM_TO_CFG(MODE_1080P_1920_1080_30fps_336M_1LANE_MIPI, cfg_1080p_30fps_336M_1lane_mipi_) },
	};
	config_awb_t const awbs[] =
	{
			{ MAP_ENUM_TO_CFG(AWB_DISABLED, cfg_disable_awb_) },
			{ MAP_ENUM_TO_CFG(AWB_SIMPLE, cfg_simple_awb_) },
			{ MAP_ENUM_TO_CFG(AWB_ADVANCED, cfg_advanced_awb_) }
	};
}

class OV5647 {
public:
	class HardwareError;

	OV5647(I2C_Client& iic, GPIO_Client& gpio) :
		iic_(iic), gpio_(gpio)
	{
		reset();
		init();
	}

	void init()
	{
		uint8_t id_h, id_l;
		readReg(reg_ID_h, id_h);
		readReg(reg_ID_l, id_l);
		if (id_h != dev_ID_h_ || id_l != dev_ID_l_)
		{
			/* Does not work. https://www.xilinx.com/support/answers/64193.html
	      std::stringstream ss;
	      ss << "Got " << std::hex << id_h << id_l << ". Expected " << dev_ID_h_ << dev_ID_l_;
			 */
			char msg[100];
			snprintf(msg, sizeof(msg), "Got %02x %02x. Expected %02x %02x\r\n", id_h, id_l, dev_ID_h_, dev_ID_l_);
			throw HardwareError(HardwareError::WRONG_ID, msg);
		}
		//[1]=0 System input clock from pad; Default read = 0x11
		writeReg(0x3103, 0x11);
		//[7]=1 Software reset; [6]=0 Software power down; Default=0x02
		writeReg(0x0103, 0x01);

		usleep(1000000);

		size_t i;
		for (i=0;i<sizeof(OV5647_cfg::cfg_init_)/sizeof(OV5647_cfg::cfg_init_[0]); ++i)
		{
			writeReg(OV5647_cfg::cfg_init_[i].addr, OV5647_cfg::cfg_init_[i].data);
		}

		//Stay in power down
	}

	Errc reset()
	{
		//Power cycle
		gpio_.clearBit(gpio_.Bits::CAM_GPIO0);
		usleep(1000000);
		gpio_.setBit(gpio_.Bits::CAM_GPIO0);
		usleep(1000000);

		return OK;
	}

	Errc set_mode(OV5647_cfg::mode_t mode)
	{
		if (mode >= OV5647_cfg::mode_t::MODE_END)
			return ERR_LOGICAL;

		auto cfg_mode = &OV5647_cfg::modes[mode];
		writeConfig(cfg_mode->cfg, cfg_mode->cfg_size);

		return OK;
	}

	Errc set_awb(OV5647_cfg::awb_t awb)
	{
		if (awb >= OV5647_cfg::awb_t::AWB_END)
			return ERR_LOGICAL;

		auto cfg_mode = &OV5647_cfg::awbs[awb];
		writeConfig(cfg_mode->cfg, cfg_mode->cfg_size);

		return OK;
	}

	Errc set_isp_format(OV5647_cfg::isp_format_t isp)
	{
		if (isp >= OV5647_cfg::isp_format_t::ISP_END)
			return ERR_LOGICAL;

		switch (isp)
		{
			case OV5647_cfg::isp_format_t::ISP_RGB:
				writeReg(OV5647_cfg::OV5647_FORMAT_MUX_CONTROL, 0x01);
				break;
			case OV5647_cfg::isp_format_t::ISP_RAW:
				writeReg(OV5647_cfg::OV5647_FORMAT_MUX_CONTROL, 0x03);
				break;
			default:
				break;
		}

		return OK;
	}

	~OV5647() { }
	void set_test(OV5647_cfg::test_t test)
	{
		switch(test)
		{
			case OV5647_cfg::test_t::TEST_DISABLED:
				writeReg(OV5647_cfg::OV5647_REG_PRE_ISP_TEST_SET1, 0x00);
				break;
			case OV5647_cfg::test_t::TEST_EIGHT_COLOR_BAR:
				writeReg(OV5647_cfg::OV5647_REG_PRE_ISP_TEST_SET1, 0x80);
				break;
			default:
				break;
		}
	}
	void readReg(uint16_t reg_addr, uint8_t& buf)
	{
		for(auto retry_count = retry_count_; retry_count > 0; --retry_count)
		{
			try
			{
				auto buf_addr = std::vector<uint8_t>{(uint8_t)(reg_addr>>8), (uint8_t)reg_addr};
				iic_.write(dev_address_, buf_addr.data(), buf_addr.size());
				iic_.read(dev_address_, &buf, 1);
				break; //If no exceptions, no mo retries
			}
			catch (I2C_Client::TransmitError const& e)
			{
				if (retry_count > 0)
				{
					continue;
				}
				else
				{
					throw HardwareError(HardwareError::IIC_NACK, e.what());
				}
			}
		}
	}
	void writeReg(uint16_t reg_addr, uint8_t const reg_data)
	{
		for(auto retry_count = retry_count_; retry_count > 0; --retry_count)
		{
			try
			{
				auto buf = std::vector<uint8_t>{(uint8_t)(reg_addr>>8), (uint8_t)reg_addr, reg_data};
				iic_.write(dev_address_, buf.data(), buf.size());
				break; //If no exceptions, no mo retries
			}
			catch (I2C_Client::TransmitError const& e)
			{
				if (retry_count > 0) continue;
				else throw HardwareError(HardwareError::IIC_NACK, e.what());
			}
		}
	}

	void writeRegLiquid(uint8_t const reg_data)
		{
		// OV5647 doesn't have liquid cam
			char msg2[100];
			snprintf(msg2, sizeof(msg2), "OV5647 doesn't have liquid cam, skip and return.");
			return;
		}
	class HardwareError : public std::runtime_error
	{
	public:
		using Errc = enum {WRONG_ID = 1, IIC_NACK};
		HardwareError(Errc errc, char const* msg) : std::runtime_error(msg), errc_(errc) {}
		Errc errc() const { return errc_; }
	private:
		Errc errc_;
	};
private:
	void usleep(uint32_t time)
	{//TODO couldn't think of anything better
		for (uint32_t i=0; i<time; i++) ;
	}
	void writeConfig(OV5647_cfg::config_word_t const* cfg, size_t cfg_size)
	{
		for (size_t i=0; i<cfg_size; ++i)
		{
			writeReg(cfg[i].addr, cfg[i].data);
		}
	}
private:
	I2C_Client& iic_;
	GPIO_Client& gpio_;
	// uint8_t dev_address_ = (0x78 >> 1); // OV5647
	uint8_t dev_address_ = (0x6C >> 1); //OV5647
	// uint8_t dev_address2_ = (0x46 >> 1); //OV5647
	uint8_t const dev_ID_h_ = 0x56;
	// uint8_t const dev_ID_l_ = 0x40; //OV5647
	uint8_t const dev_ID_l_ = 0x47; //OV5647
	uint16_t const reg_ID_h = 0x300A;
	uint16_t const reg_ID_l = 0x300B;
	unsigned int const retry_count_ = 10;
};

} /* namespace digilent */

#endif /* OV5647_H_ */
