#include "cmos_time.h"

#define BINARY_MODE_BIT 0x04
#define H_FORMAT_BIT 0x02

int cmos_bcd_to_decimal(int bcd)
{
	return ((bcd>>4)*10)+bcd%16;
}


inline void fill_cmos_tstruct(cmos_time_struct * tstruct) 
{
	uint32 IntBuffer;
	uint8 h_fmt = 0;
	uint8 mode = 0;
	char TextBuffer[128];

	uint8 temp_seconds, temp_minutes, temp_hours, temp_day, temp_dayofweek, temp_month, temp_year, temp_century; 

	IntBuffer = CMOS_READ(0xb);
	
	if ((IntBuffer & BINARY_MODE_BIT) == BINARY_MODE_BIT) 
	{ 
		//Binary mode = true
		mode = 1;
	} 
	else 
	{
	        //BCD (Binary coded decimal) mode
		mode = 0;
	}

	if ((IntBuffer & H_FORMAT_BIT) == H_FORMAT_BIT) 
	{
		h_fmt = 1;
	} 
	else 
	{
		h_fmt = 0;
	}

	//Ok so now we know the format that the date and time are gohna be in.
	
	//Handle seconds
	temp_seconds = CMOS_READ(0);

	if (mode == 1)
	{
		//Seconds are already in binary
		tstruct->seconds = temp_seconds;
	} 
	else 
	{
		tstruct->seconds = cmos_bcd_to_decimal(temp_seconds);
	}

	//Handle minutes
	temp_minutes = CMOS_READ(2);
	if (mode == 1) 
	{
		//Mins are already in binary
		tstruct->minutes = temp_minutes;
	}
	else 
	{
		tstruct->minutes = cmos_bcd_to_decimal(temp_minutes);
	}

	//Handle hours
	//This is the only one affected by the h_fmt bit.
	//If its set to true we don't need to do anything
	//If its set to false we need to check the 0x80 bit if its false then its am if its true then its pm
	temp_hours = CMOS_READ(4);
	if (mode == 1) 
	{
		if (h_fmt == 0) 
		{
			//Binary
			if ((temp_hours & 0x80) == 0x80) 
			{ 	
				//PM
				temp_hours = temp_hours ^ 0x80;
				temp_hours += 12;
			}
		}
		
		tstruct->hours = temp_hours;
	}
	else 
	{
		//BCD
		if (h_fmt == 0) 
		{
			int pm = 0;

			if ((temp_hours & 0x80) == 0x80) 
			{
				//Its PM
				pm = 1;
				temp_hours = temp_hours ^ 0x80;			
			}

			temp_hours = cmos_bcd_to_decimal(temp_hours);
			if (pm == 1) temp_hours += 12;
		}
		else
		{
			temp_hours = cmos_bcd_to_decimal(temp_hours);		
		}

		tstruct->hours = temp_hours;
	}

	//Handle day of the week
	temp_dayofweek = CMOS_READ(6);

	if (mode == 1) 
	{
		//already in binary
		tstruct->dayofweek = temp_dayofweek;
	} 
	else 
	{
		tstruct->dayofweek = cmos_bcd_to_decimal(temp_dayofweek);
	}

	//Handle days
	temp_day = CMOS_READ(7);
	if (mode == 1)
	{
		//already in binary
		tstruct->day = temp_day;
	}
	else 
	{
		tstruct->day = cmos_bcd_to_decimal(temp_day);
	}

	//Handle month
	temp_month = CMOS_READ(8);
	if (mode == 1) 
	{
		//already in binary
		tstruct->month = temp_month;
	}
	else 
	{
		tstruct->month = cmos_bcd_to_decimal(temp_month);
	}

	//Handle year
	temp_year = CMOS_READ(9);
	if (mode == 1) 
	{
		//already in binary
		tstruct->year = temp_year;
	} 
	else 
	{
		tstruct->year = cmos_bcd_to_decimal(temp_year);
	}

	//Handle century
	temp_century = CMOS_READ(0x32);
	if (mode == 1) 
	{
		//already in binary
		tstruct->century = temp_century;
	} 
	else 
	{
		tstruct->century = cmos_bcd_to_decimal(temp_century);
	}

	//Handle seconds
	temp_seconds = CMOS_READ(0);
	if (mode == 1) 
	{
		//Seconds are already in binary
		tstruct->seconds = temp_seconds;
	} 
	else 
	{
		tstruct->seconds = cmos_bcd_to_decimal(temp_seconds);
	}

	return;
}
