#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LOVE PT_LOVE 94
Element_LOVE::Element_LOVE()
{
    Identifier = "DEFAULT_PT_LOVE";
    Name = "LOVE";
    Colour = PIXPACK(0xFF30FF);
    MenuVisible = 1;
    MenuSection = SC_CRACKER2;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.00f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.0f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 0;
    
    Weight = 100;
    
    Temperature = 373.0f;
    HeatConduct = 40;
    Description = "Love...";
    
    State = ST_GAS;
    Properties = TYPE_SOLID;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_LOVE::update;
    Graphics = NULL;
}

//#TPT-Directive ElementHeader Element_LOVE static int update(UPDATE_FUNC_ARGS)
int Element_LOVE::update(UPDATE_FUNC_ARGS)
 {
	/*int t = parts[i].type;
	if (t==PT_LOVE)
		ISLOVE=1;
	else if (t==PT_LOLZ)
		ISLOLZ=1;
	else if (t==PT_GRAV)
		ISGRAV=1;*/
	return 0;
}


Element_LOVE::~Element_LOVE() {}