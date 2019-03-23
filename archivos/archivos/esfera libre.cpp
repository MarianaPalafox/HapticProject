/********************************************
*
*	Programa ejemplo, inicializaci�n del dispositivo h�ptico
*   y creaci�n de mundo virtual simple
*	Taller de aplicaciones con h�pticos 
*	usando chai3d
*
*   Autor: Eusebio Ric�rdez V�zquez
*********************************************/

#include "stdafx.h"
#include "chai3d.h"

//------------------------------------------------------------
//Definici�n de variables globales
// El mundo que contendr� todo
cWorld* world;

// La camara a trav�s de la cual observaremos el mundo
cCamera* camera;

// Fuente de luz
cLight *light;

// Ancho y alto de la pantalla gr�fica
int displayW  = 0;
int displayH  = 0;

// Dimensiones que tendr� la pantalla gr�fica
const int WINDOW_SIZE_W         = 600;
const int WINDOW_SIZE_H         = 600;

// Opciones para el menu del rat�n 
const int OPTION_FULLSCREEN     = 1;
const int OPTION_WINDOWDISPLAY  = 2;

// Manejador del dispositivo h�ptico
cHapticDeviceHandler *handler;

// Dispositivo h�ptico
cGenericHapticDevice* hapticDevice;

// Una esfera
cShapeSphere* object0;

// Variable de control para la simulaci�n
bool simulationRunning = false;

// Variable para detectar el fin de la simulaci�n
bool simulationFinished = false;

//-- Factor de fuerza
double deviceForceScale;

// Factor de escala entre el dispositivo y el mundo
double workspaceScaleFactor;

// Espacio de trabajo del dispositivo haptico
double cursorWorkspaceRadius;

// Esfera para explorar el mundo
cShapeSphere* device;

// Radio de la esfera exploradora
double deviceRadius;

// Radio del objeto virtual
double objectRadius;



//---------------------------------------------------------------------------
// Prototipos de funciones
//---------------------------------------------------------------------------

// funci�n de resize para OpenGL
void resizeWindow(int w, int h);

// Detecci�n de teclas 
void keySelect(unsigned char key, int x, int y);

// Menu para el mouse
void menuSelect(int value);

// Funci�n para terminar las ejecuciones antes de cerrar
void close(void);

// Rutina gr�fica principal
void updateGraphics(void);

// Rutina principal h�ptica
void updateHaptics(void);

// C�lculo de colisiones
bool calculaCollission(cVector3d pos);

// C�lculo de fuerzas 
cVector3d calculaFuerza(cVector3d fuerza,  cVector3d pos, cVector3d velocidad);

// C�lculo del proxy point
cVector3d calculaProxy(cVector3d pos);


int main(int argc, char* argv[])
{
//-----------------------------------------------------------------------
// INICIALIZACION
//-----------------------------------------------------------------------

    printf ("\n");
    printf ("-----------------------------------\n");
    printf ("Efectos propios\n");
    printf ("Especialidad en Simuladores gr�ficos\n");
    printf ("Esfera haptica\n\n");
	printf ("Eusebio Ricardez Vazquez\n");
	printf ("-----------------------------------\n");
    printf ("[x] o [esc] para terminar el programa\n\n");
   
  // Creaci�n y configutaci�n del mindo virtual
    world = new cWorld();

    // Elecci�n del color de fondo
    // el color se selecciona usando componentes (R,G,B) 
    world->setBackgroundColor(0.0, 0.0, 0.0);

    // Creacion y configuraci�n de la c�mara
    camera = new cCamera(world);
    world->addChild(camera);

    // posicion y orientaci�n de la camara
    camera->set( cVector3d (5.0, 0.0, 0.0),    // position (eye)
                 cVector3d (0.0, 0.0, 0.0),    // lookat position (target)
                 cVector3d (0.0, 0.0, 1.0));   // direcci�n del vector "up" 

    // Planos de corte "near y far" de la c�mara
    // lo que est� atras o adelante de estos planos no se visualizar�
    camera->setClippingPlanes(0.01, 10.0);

    // Habilitaci�n de transparencia
    camera->enableMultipassTransparency(true);

    // Creaci�n de una fuente de luz
    light = new cLight(world);
    camera->addChild(light);                   // Fijar la luz a la c�mara
    light->setEnabled(true);                   // Habilitar fuente de luz
    light->setPos(cVector3d( 3.0, 1.5, 2.0));  // Posici�n de la fuente de luz
    light->setDir(cVector3d(-2.0, 0.5, 1.0));  // Direcci�n del haz de luz

// Configuraci�n del dispositivo h�ptico y la esfera de exploraci�n h�ptica

	// Activaci�n del controlador del dispositivo h�ptico
	handler = new cHapticDeviceHandler();
	// Accede al primer dispositivo disponible
//	cGenericHapticDevice* hapticDevice;
	if(handler->getDevice(hapticDevice, 0) < 0)
	{
		printf("Error: no hay dispositivo disponible");
		exit(1);
	}
	// Obtener la informaci�n del dispositivo conectado
	cHapticDeviceInfo info;
	if (hapticDevice != NULL)
	{
		hapticDevice->open();
		hapticDevice->initialize();
		info = hapticDevice->getSpecifications();
	}

  // Espacio de trabajo del dispositivo haptico
    cursorWorkspaceRadius = 1.5;

    // Factor de ajuste entre el espacio de trabajo del dispositivo haptico
    // y el mundo virtual
    workspaceScaleFactor = cursorWorkspaceRadius / info.m_workspaceRadius;

    // Factor de fuerza que se env�a al dispositivo haptico
   
    deviceForceScale = 2 * info.m_maxForce;

	// Radio de la herramienta de exploraci�n
	deviceRadius = 0.08;

	// Definici�n de la herramienta de exploraci�n
    device = new cShapeSphere(deviceRadius);
    world->addChild(device);
    device->m_material.m_ambient.set(1.0, 1.0, 1.0, 0.5);
    device->m_material.m_diffuse.set(1.0, 1.0, 1.0, 0.5);
    device->m_material.m_specular.set(1.0, 1.0, 1.0, 0.5);
    device->m_material.setShininess(100);
    device->setUseTransparency(false);
	
    
	// Creaci�n del objeto virtual que ser� explorado
	cShapeSphere* object;
	objectRadius = 0.75;
	object = new cShapeSphere(objectRadius);
    
	
    // Propiedades de color
	
    object->m_material.m_ambient = cColorf(0.2, 0.0, 0.0);
    object->m_material.m_diffuse = cColorf(0.7, 0.1, 0.2);
	object->setUseTransparency(false);
    
    world->addChild(object);
    

	 //-----------------------------------------------------------------------
    // OPEN GL
    //-----------------------------------------------------------------------

    // inicializacion de  GLUT
    glutInit(&argc, argv);

    // obtenci�n de la resoluci�n actual de la pantala y 
	// ubicacion del gr�fico en el centro de la misma
    int screenW = glutGet(GLUT_SCREEN_WIDTH);
    int screenH = glutGet(GLUT_SCREEN_HEIGHT);
    int windowPosX = (screenW - WINDOW_SIZE_W) / 2;
    int windowPosY = (screenH - WINDOW_SIZE_H) / 2;

    // inicializacion de glut
    glutInitWindowPosition(windowPosX, windowPosY);
    glutInitWindowSize(WINDOW_SIZE_W, WINDOW_SIZE_H);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(updateGraphics);
    glutKeyboardFunc(keySelect);
    glutReshapeFunc(resizeWindow);
    glutSetWindowTitle("Ejemplo");    // titulo de la ventana OpenGL

    // Creaci�n de menu para el mouse
    glutCreateMenu(menuSelect);
    glutAddMenuEntry("Pantalla completa", OPTION_FULLSCREEN);
    glutAddMenuEntry("Ventana", OPTION_WINDOWDISPLAY);
    glutAttachMenu(GLUT_RIGHT_BUTTON);


    //-----------------------------------------------------------------------
    // Iniciar simulaci�n
    //-----------------------------------------------------------------------

    // inicio de la simulaci�n
    simulationRunning = true;

    // Creaci�n del hilo para la parte h�ptica
    cThread* hapticsThread = new cThread();
    hapticsThread->set(updateHaptics, CHAI_THREAD_PRIORITY_HAPTICS);

    // loop principal de GLUT
    glutMainLoop();

    // Verificar que todo este cerrado cuando se cierra la ventana grafica
    close();

    // salida
    return (0);

}

//---------------------------------------------------------------------------

void resizeWindow(int w, int h)
{
    // Redibuja la ventana gr�fica con el nuevo tama�o
    displayW = w;
    displayH = h;
    glViewport(0, 0, displayW, displayH);
}

//---------------------------------------------------------------------------

void keySelect(unsigned char key, int x, int y)
{
    if ((key == 27) || (key == 'x'))
    {
        // verifica que todo este cerrado
        close();

        // termina la aplicacion
        exit(0);
    }
}

//---------------------------------------------------------------------------

void menuSelect(int value)
{
    switch (value)
    {
        // Pantalla completa
        case OPTION_FULLSCREEN:
            glutFullScreen();
            break;

        // tama�o original
        case OPTION_WINDOWDISPLAY:
            glutReshapeWindow(WINDOW_SIZE_W, WINDOW_SIZE_H);
            break;
    }
}

//---------------------------------------------------------------------------

void close(void)
{
    // detener la simulacion, esta variable controla el hilo haptico
    simulationRunning = false;

    // espera 100 ms para asegurarse que el hilo haptico se cerr�
    while (!simulationFinished) { cSleepMs(100); }

    // detener el dispositivo haptico
	hapticDevice->close();
}

//---------------------------------------------------------------------------

void updateGraphics(void)
{
    // trazar (render) el mundo
    camera->renderView(displayW, displayH);

    
    glutSwapBuffers();

    // verifica errores de glut
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) printf("Error:  %s\n", gluErrorString(err));

    // Dibujar el mundo
    if (simulationRunning)
    {
        glutPostRedisplay();
    }


}

//---------------------------------------------------------------------------

void updateHaptics(void)
{
	// main haptic simulation loop
	cVector3d fuerza,pos, proxy, velLineal;
    
    while(simulationRunning)
    {
        // compute global reference frames for each object
        world->computeGlobalPositions(true);
		int i;
		i = hapticDevice->getPosition(pos);
		proxy = calculaProxy(pos);
        pos.mul(workspaceScaleFactor);
		hapticDevice->getLinearVelocity(velLineal);
        device->setPos(pos);
		//device->setPos(proxy);
      	
		
		
		// Detecta si hay colisiones
		if (calculaCollission(pos))
		{
			// Si hay colisiones determina la fuerza
			fuerza = calculaFuerza(fuerza,  pos, velLineal);
			device->setPos(proxy);
		}
		else
		{
			device->setPos(pos);

			bool amortiguamiento = false;

			if(amortiguamiento)
			{
				double viscosidad = 2.0;
				fuerza = viscosidad * velLineal;
			}
			else
				fuerza.zero();		
		}
		//device->setPos(pos);

		// Calcula la fuerza y env�ala al dispositivo
		fuerza *= deviceForceScale;
		hapticDevice->setForce(fuerza );
    }
    
    // exit haptics thread
    simulationFinished = true;
}

cVector3d calculaProxy(cVector3d pos)
{
    double t;
	cVector3d proxy;
	double radioObjeto = objectRadius+deviceRadius;
	t = radioObjeto / pos.length();
	proxy = cVector3d((t*pos.x),(t*pos.y),(t*pos.z));
	return proxy;
}

bool calculaCollission(cVector3d pos)
{
	// Calculo de la colisi�n con la esfera virtual
	bool ret = false;
	// Se considera el tama�o de la herramienta mejorar la simulaci�n
	double radioObjeto = objectRadius+deviceRadius;
	if (pos.length() <= radioObjeto)
		ret = true;
    return ret;
}

cVector3d calculaFuerza(cVector3d fuerza,  cVector3d pos, cVector3d velocidad)
{
		// C�lculo de la fuerza que se ejerce, se considera el radio de la herramienta de exploraci�n
		double radioObjeto = objectRadius+deviceRadius;
		// c�lculo de la distancia de la herramienta al origen
		double distancia = pos.length();
		double viscosidad = 2.0;
		// C�lculo de magnitud y direcci�n del vector de fuerza
		fuerza.x = pos.x/distancia*(radioObjeto-distancia);
		fuerza.y = pos.y/distancia* (radioObjeto-distancia);
		fuerza.z = pos.z/distancia* (radioObjeto-distancia);
//		fuerza.x = pos.x /distancia * (radioObjeto-distancia)- viscosidad * velocidad.x; 
//		fuerza.y = pos.y /distancia * (radioObjeto-distancia)- viscosidad * velocidad.y; 
//		fuerza.x = pos.z /distancia * (radioObjeto-distancia)- viscosidad * velocidad.z; 
//		fuerza.x = pos.x * (radioObjeto-distancia)- viscosidad * velocidad.x; 
//		fuerza.y = pos.y  * (radioObjeto-distancia)- viscosidad * velocidad.y; 
//		fuerza.x = pos.z  * (radioObjeto-distancia)- viscosidad * velocidad.z; 



		return fuerza;
}
//---------------------------------------------------------------------------
