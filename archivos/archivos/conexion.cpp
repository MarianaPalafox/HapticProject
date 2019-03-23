/********************************************
*
*	Programa ejemplo, inicializaci�n del dispositivo h�ptico
*	Taller de aplicaciones con h�pticos 
*	usando chai3d
*
*   Autor: Eusebio Ric�rdez V�zquez
*********************************************/

#include "stdafx.h"
#include "chai3d.h"
cHapticDeviceHandler *handler;
int main()
{
handler = new cHapticDeviceHandler();
// Conectarse al primer dispositivo disponible
cGenericHapticDevice* hapticDevice;
//handler->getDevice(hapticDevice, 0);
if(handler->getDevice(hapticDevice, 0) < 0)
	{
		printf("Error: no hay dispositivo disponible");
		exit(1);
	}
// Obtener informaci�n del dispositivo
cHapticDeviceInfo info;
if (hapticDevice)
{
	hapticDevice->open();
	hapticDevice->initialize();
	info = hapticDevice->getSpecifications();
}

cVector3d posicion,posicionAnt;
int i;

while(true)
{
	i = hapticDevice->getPosition(posicion);
	if(!posicion.equals(posicionAnt,0.0))
	{
		printf("posicion: x= %5.4f, y= %5.4f,z= %5.4f \n",posicion.x,posicion.y,posicion.z);
		//posicion.print();
	}
	posicionAnt = posicion;
}


}

