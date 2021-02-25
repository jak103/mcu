/*
 * radio.c
 *
 * Created: 12/27/2020 8:59:36 PM
 *  Author: jacob
 */ 

#include <stdint.h>
#include <spi_basic.h>
#include <atmel_start_pins.h>
#include <stdio.h>
#include <util/delay.h>

#include <state.h>

#define MRF89_SPI_START      0x00
#define MRF89_SPI_READ       0x40
#define MRF89_SPI_ADDR_SHIFT 0x01
#define MRF89_SPI_WRITE      0x00
#define MRF89_SPI_STOP       0x00

#define MRF89_XTAL_FREQ 12.8

#define MRF89_REG_GCON    0x00
#define MRF89_REG_DMOD    0x01
#define MRF89_REG_FDEV    0x02
#define MRF89_REG_BRS     0x03
#define MRF89_REG_FLTH    0x04
#define MRF89_REG_FIFOC   0x05
#define MRF89_REG_R1C     0x06
#define MRF89_REG_P1C     0x07
#define MRF89_REG_S1C     0x08
#define MRF89_REG_R2C     0x09
#define MRF89_REG_P2C     0x0A
#define MRF89_REG_S2C     0x0B
#define MRF89_REG_PAC     0x0C
#define MRF89_REG_FTXRXI  0x0D
#define MRF89_REG_FTPRI   0x0E
#define MRF89_REG_RSTHI   0x0F
#define MRF89_REG_FILC    0x10
#define MRF89_REG_PFC     0x11
#define MRF89_REG_SYNC    0x12
#define MRF89_REG_RESV    0x13
#define MRF89_REG_RSTS    0x14
#define MRF89_REG_OOKC    0x15
#define MRF89_REG_SYNCV31 0x16
#define MRF89_REG_SYNCV23 0x17
#define MRF89_REG_SYNCV15 0x18
#define MRF89_REG_SYNCV07 0x19
#define MRF89_REG_TXCON   0x1A
#define MRF89_REG_CLKO    0x1B
#define MRF89_REG_PLOAD   0x1C
#define MRF89_REG_NADDS   0x1D
#define MRF89_REG_PKTC    0x1E
#define MRF89_REG_FCRC    0x1F

// GCON values (Page 30, 2.14.1)
#define MRF89_MODE_SLEEP    0x00 // (0x0 << 5)
#define MRF89_MODE_STANDBY  0x20 // (0x1 << 5)
#define MRF89_MODE_SYNTH    0x40 // (0x2 << 5)
#define MRF89_MODE_RECEIVE  0x60 // (0x3 << 5)
#define MRF89_MODE_TRANSMIT 0x80 // (0x4 << 5) 
#define MRF89_FREQ_902_915  0x00
#define MRF89_FREQ_915_928  0x08
#define MRF89_FBS_MASK      0x18

#define MRF89_VCOT_60MV     0x02

// DMOD values (Page 31, 2.14.2)
#define MRF89_MOD_FSK       0x80
#define MRF89_OPMODE_PACKET 0x04
#define MRF89_GAIN_MAX      0x00

// FDEV values (Page 32, 2.14.3)
#define MRF89_FDEV_400kHz 0x00
#define MRF89_FDEV_33kHz  0x0B

// Bit rates (Page 32, 2.14.4)
#define MRF89_RATE_50kbps 0x03 

// FIFO config reg (Page 33, 2.14.6)
#define MRF89_FIFOSIZE_64     0xc0
#define MRF89_FIFO_INT_THRESH_16 0x10

// FIFO Transmit and receive interrupt configuration (Page 38, 2.15.1)
#define MRF89_IRQ0RXS_PACKET_PAYLOAD_READY 0x00
#define MRF89_IRQ0RXS_PACKET_SYNC          0xC0
#define MRF89_IRQ1RXS_PACKET_CRCOK         0x00
#define MRF89_IRQ1TX_PACKET_TXDONE         0x08


// FIFO transmit pll and rssi interrupt configuration (Page 40, 2.15.2)
#define MRF89_DISABLE_PLOCK_PIN 0x00

// Filter conifiguration (Page 42, 2.16.1)
#define MRF89_PASFILV_414kHz 0xB0 
#define MRF89_BUTFILV_150kHz 0x05

// SYNC control (Page 44, 2.16.3)
#define MRF89_SYNCREN_ON        0x20
#define MRF89_SYNC_WORD_SIZE_32 0x18 

// SYNC values (Page 47, 2.17)
#define MRF89_SYNC_VALUE_31 0x69
#define MRF89_SYNC_VALUE_23 0x81
#define MRF89_SYNC_VALUE_15 0x7E
#define MRF89_SYNC_VALUE_07 0x96

// Transmit configuration (Page 49, 2.18.1)
#define MRF89_TXIPOLFV_75kHz   0x20
#define MRF89_TX_POWER_13dBm   0x00
#define MRF89_TX_POWER_10dBm   0x02
#define MRF89_TX_POWER_7dBm    0x04
#define MRF89_TX_POWER_4dBm    0x06
#define MRF89_TX_POWER_1dBm    0x08
#define MRF89_TX_POWER_neg2dBm 0x0A
#define MRF89_TX_POWER_neg5dBm 0x0C
#define MRF89_TX_POWER_neg8dBm 0x0E

// Clock output control (Page 50, 2.19.1)
#define MRF89_CLOCK_OFF 0x00

// Payload configuration (Page 51, 2.20.1)
#define MRF89_MANCH_OFF  0x00
#define MRF89_PLD_LEN_16 0x10  

// Packet configuration (Page 52, 2.20.3)
#define MRF89_PKT_LEN_FIXED 0x00
#define MRF89_PREAMBLE_4    0x60
#define MRF89_WHITEN_ON     0x10
#define MRF89_CRC_ON        0x08
#define MRF89_ADDR_FILT_OFF 0x00

// FIFO CRC configuration (Page 53, 2.20.4)
#define MRF89_AUTO_CLEAR_CRC_ON 0x00
#define MRF89_FIFO_STDBY_ON     0x00

uint8_t radioMode = MRF89_MODE_STANDBY; // Initial mode

void RegisterSet(uint8_t address, uint8_t value)
{
	RADIO_CS_CONFIG_set_level(0); // Select the CONFIG SPI block
	uint8_t temp = MRF89_SPI_START | MRF89_SPI_WRITE | address << MRF89_SPI_ADDR_SHIFT | MRF89_SPI_STOP;
	SPI_0_write_byte(temp);
	SPI_0_write_byte(value);
	RADIO_CS_CONFIG_set_level(1);
}
    
uint8_t RegisterRead(uint8_t address)
{
	RADIO_CS_CONFIG_set_level(0); // Select the CONFIG SPI block
	uint8_t temp = MRF89_SPI_START | MRF89_SPI_READ | address << MRF89_SPI_ADDR_SHIFT | MRF89_SPI_STOP;
	SPI_0_write_byte(temp);
	uint8_t value = SPI_0_read_byte();
	RADIO_CS_CONFIG_set_level(1);
        
	return value;
}

void radioSetMode(uint8_t mode)
{
	uint8_t gcon = RegisterRead(MRF89_REG_GCON);
	gcon &= 0x1F; // clear first three bits
	gcon |= mode;
	
	RegisterSet(MRF89_REG_GCON, gcon);
	_delay_ms(6); // delay to allow mode transitions, Page 91, 3.13 
}

void SendData(uint8_t* packet, uint8_t length)
{
	currentState = RF_TRANSMITTING;
	radioSetMode(MRF89_MODE_TRANSMIT);
	
	// Done with a for loop because it is required to toggle the CS_DATA line between each byte
	for (uint8_t i = 0; i < length; ++i)
	{
		RADIO_CS_DATA_set_level(0);
		SPI_0_write_byte(packet[i]);
		RADIO_CS_DATA_set_level(1);	
	}
	
	// TODO: Optimize this from blocking to useing ISRs and sleeping
	while (currentState == RF_TRANSMITTING);
	
	radioSetMode(MRF89_MODE_RECEIVE);
	
	while (currentState == RF_WAIT_FOR_REPLY);
}

uint8_t ReadData(uint8_t* packet, uint8_t maxLength)
{	
	// Read length
	for (uint8_t i = 0; i < maxLength; ++i)
	{
		RADIO_CS_DATA_set_level(0);
		packet[i] = SPI_0_read_byte();
		RADIO_CS_DATA_set_level(1);
	}
		
	return 16;
}

void readConfig(void)
{
	for (uint8_t reg = 0x00; reg <= 0x1F; reg++)
	{
		uint8_t result = RegisterRead(reg);
		printf("Reg: %x - %x\r\n", reg, result);
	}
}

void radioReset(void)
{
	RADIO_RESET_set_level(1);
	_delay_ms(1); // Page 15, section 2.2
	RADIO_RESET_set_level(0);
	_delay_ms(6); // Page 15, section 2.2
}

void radioSetFrequency(float center)
{	
	uint8_t FBS = MRF89_FREQ_902_915;
	if (center >= 902.0 && center < 915.0)
	{
		FBS = MRF89_FREQ_902_915;
	}
	else if (center >= 915.0 && center <= 928.0)
	{
		FBS = MRF89_FREQ_915_928;
	}
	

	// Based on frequency calcs done in MRF89XA.h
	//    uint8_t R = 100; // Recommended
	uint8_t R = 119; // Also recommended :-(
	uint32_t center_kHz = center * 1000;
	uint32_t xtal_kHz = (MRF89_XTAL_FREQ * 1000);
	uint32_t compare = (center_kHz * 8 * (R + 1)) / (9 * xtal_kHz);
	uint8_t P = ((compare - 75) / 76) + 1;
	uint8_t S = compare - (75 * (P + 1));

	// Now set the new register values:
	uint8_t val = RegisterRead(MRF89_REG_GCON);
	val = (val & ~MRF89_FBS_MASK) | (FBS & MRF89_FBS_MASK);
	RegisterSet(MRF89_REG_GCON, val);

	RegisterSet(MRF89_REG_R1C, R); 
	RegisterSet(MRF89_REG_P1C, P); 
	RegisterSet(MRF89_REG_S1C, S); 
}

void radioInit(void)
{
	radioReset();
	/* From data sheet
	Initialization in 16 steps!
	1. In the GCONREG register:
		a) Set the Chip Mode (CMOD<2:0>), Frequency Band (FBS<1:0>), and VCO Trim (VCOT<1:0>) bits.
		b) Program the Frequency band.
		c) Set the Trim bits to appropriately tune in the	VCO.
	2. In the DMODREG register:
		a) Select the Modulation Type using the MODSEL<1:0> bits.
		b) Enable DATA mode for Transmission using the DMODE0 and DMODE1 bits.
		c) Select the gain for IF chain using the	IFGAIN<1:0> bits.
		d) In the FDEVREG register, program the Frequency	Deviation bits (FDVAL<7:0>).
	3. In the BRSREG register, program the Bit Rate	using the BRVAL<6:0> bits.
	4. In the FLTHREG register, set the Floor Threshold	for OOK using the FTOVAL<7:0> bits.
	5. In the FIFOCREG register, configure the FIFO	Size and FIFO Threshold using the FSIZE<1:0> and FTINT<5:0> bits.
	6. In the PACREG register, configure the Power Amplifier Ramp Control using the PARC<1:0>	bits.
	7. In the FTXRXIREG register:
		a) Configure the RX interrupts for IRQ0 and	IRQ1 using the IRQ0RXS<1:0> and	IRQ1RXS<1:0> bits.
		b) Configure the TX interrupts for IRQ1 using	the IRQ1TX bit.
	8. In the FTPRIREG register:
		a) Configure the TX interrupts for IRQ0 using	the IRQ0TXST bit.
		b) Enable PLL Lock for interrupt on IRQ1 using the LENPLL bit.
	9. In the RSTHIREG, program the RSSI Threshold value for interrupt request using the RTIVAL<7:0> bits.
	10. In the FILCREG register, enable the Passive	Filter using the PASFILV<3:0> bits.
	11. Configure RX parameters:
		a) Enable Passive Filter with the value as set in step 10.
		b) Set fc and fo.
		c) Enable SYNC and Set SYNC Word, Size,	Length, and Tolerance.
		d) Set configuration bytes for OOK Threshold from OOKCREG.
	12. In the SYNCREG register, set SYNCWSZ<1:0> = 11 for 32-bit SYNC Word.
	13. Configure TX parameters:
		a) Change or Reset fc.
		b) In the TXCONREG register, enable TX and its transmit power using the	TXIPOLFV<3:0> and TXOPVAL<2:0> bits.
	14. In the CLKOUTREG register, configure the Clock Settings using the CLKOCNTRL and CLKOFREQ<4:0> bits.
	15. Configure the Packet Frame parameters in the PLOADREG, NADDSREG, PKTCREG, and	FCRCREG registers:
		a) Enable Manchester Encoding.
		b) Set the packet format and the length of the packet.
		c) Set the Node local address.
		d) Program preamble variables.
		e) Configure CRC parameters.
		f) Enable Address Filtering.
	16. In the FCRCREG register, enable FIFO write access using the FRWAXS bit.
	*/

	// TODO Configure pin interrupt

    // Make sure we are not in some unexpected mode from a previous run    
    radioSetMode(MRF89_MODE_STANDBY); 

    // No way to check the device type but lets trivially check there is something there
    // by trying to change a register:
    RegisterSet(MRF89_REG_FDEV, 0xaa);
    if (RegisterRead(MRF89_REG_FDEV) != 0xaa)
		{
			printf("Couldn't set a register!\r\n");
			// TODO Error handling that causes an error buzz?
			//return false;
		}
		
    RegisterSet(MRF89_REG_FDEV, 0x3); // Back to the default for FDEV
    if (RegisterRead(MRF89_REG_FDEV) != 0x3)
		{
			printf("Couldn't set a register!\r\n");
			//return false;
		}

    // When used with the MRF89XAM9A module, per 75017B.pdf section 1.3, need:
    // crystal freq = 12.8MHz
    // clock output disabled
    // frequency bands 902-915 or 915-928
    // VCOT 60mV
    // OOK max 28kbps
    // Based on 70622C.pdf, section 3.12: 
    RegisterSet(MRF89_REG_GCON, MRF89_MODE_STANDBY | MRF89_FREQ_902_915 | MRF89_VCOT_60MV);
    RegisterSet(MRF89_REG_DMOD, MRF89_MOD_FSK | MRF89_OPMODE_PACKET | MRF89_GAIN_MAX); // FSK, Packet mode, LNA 0dB
    RegisterSet(MRF89_REG_FDEV, MRF89_FDEV_33kHz); 
    RegisterSet(MRF89_REG_BRS,  MRF89_RATE_50kbps);
    //RegisterSet(MRF89_REG_FLTH, 0); // (OOK only)
    RegisterSet(MRF89_REG_FIFOC, MRF89_FIFOSIZE_64);
    //RegisterSet(MRF89_REG_R1C, 0); // TODO: Learn how to set frequency
    //RegisterSet(MRF89_REG_P1C, 0); // TODO: Learn how to set frequency
    //RegisterSet(MRF89_REG_S1C, 0); // TODO: Learn how to set frequency
    RegisterSet(MRF89_REG_R2C, 0); // Frequency set 2 not used // TODO: Send to base station on one frequency, receive on another
    RegisterSet(MRF89_REG_P2C, 0); // Frequency set 2 not used
    RegisterSet(MRF89_REG_S2C, 0); // Frequency set 2 not used
    //RegisterSet(MRF89_REG_PAC, RH_MRF89_PARC_23); // (OOK only)
    // IRQ0 rx mode: SYNC (not used)
    // IRQ1 rx mode: CRCOK
    // IRQ1 tx mode: TXDONE
    RegisterSet(MRF89_REG_FTXRXI, MRF89_IRQ0RXS_PACKET_SYNC | MRF89_IRQ1RXS_PACKET_CRCOK | MRF89_IRQ1TX_PACKET_TXDONE);
    RegisterSet(MRF89_REG_FTPRI, MRF89_DISABLE_PLOCK_PIN);
    RegisterSet(MRF89_REG_RSTHI, 0x00); // default not used if no RSSI interrupts
    RegisterSet(MRF89_REG_FILC, MRF89_PASFILV_414kHz | MRF89_BUTFILV_150kHz); 
																
    //RegisterSet(MRF89_REG_PFC, 0x38);// OOK Only 100kHz, recommended, but not used, see RH_MRF89_REG_12_SYNCREG OOK only?
		RegisterSet(MRF89_REG_SYNC, MRF89_SYNCREN_ON | MRF89_SYNC_WORD_SIZE_32); // No polyphase, no bsync, sync, 0 errors
    //RegisterSet(MRF89_REG_RSV, 0x07);// reserved, read only
    //RegisterSet(RH_MRF89_REG_14_RSTSREG, 0x00); // NO, read only
    //RegisterSet(MRF89_REG_OOKC, 0x00); // OOK only
    RegisterSet(MRF89_REG_SYNCV31, MRF89_SYNC_VALUE_31); 
    RegisterSet(MRF89_REG_SYNCV23, MRF89_SYNC_VALUE_23); 
    RegisterSet(MRF89_REG_SYNCV15, MRF89_SYNC_VALUE_15); 
    RegisterSet(MRF89_REG_SYNCV07, MRF89_SYNC_VALUE_07); 
		
		// Tx Config   
    RegisterSet(MRF89_REG_TXCON, MRF89_TXIPOLFV_75kHz | MRF89_TX_POWER_1dBm); // TX cutoff freq=75kHz, 1 dBm TODO: turn up power
    RegisterSet(MRF89_REG_CLKO, MRF89_CLOCK_OFF); // Disable clock output to save power
    RegisterSet(MRF89_REG_PLOAD, MRF89_MANCH_OFF | MRF89_PLD_LEN_16); // Manchester off, payload length 16
    RegisterSet(MRF89_REG_NADDS, 0x00); // Node Address (0=default) Not used		
		RegisterSet(MRF89_REG_PKTC, MRF89_PKT_LEN_FIXED | MRF89_PREAMBLE_4 | MRF89_WHITEN_ON | MRF89_CRC_ON | MRF89_ADDR_FILT_OFF);
    RegisterSet(MRF89_REG_FCRC, MRF89_AUTO_CLEAR_CRC_ON | MRF89_FIFO_STDBY_ON); // default (FIFO access in standby=write, clear FIFO on CRC mismatch)

    // Looking OK now
    // Set some suitable defaults:
    //setPreambleLength(3); // The default
    //uint8_t syncwords[] = { 0x69, 0x81, 0x7e, 0x96 }; // Same as RH_MRF89XA
    //setSyncWords(syncwords, sizeof(syncwords));
    //setTxPower(RH_MRF89_TXOPVAL_1DBM);
    //if (!setFrequency(915.4))
			//return false;
    //// Some slow, reliable default speed and modulation
    //if (!setModemConfig(FSK_Rb20Fd40))
			//return false;
//
    //return true;
	
}