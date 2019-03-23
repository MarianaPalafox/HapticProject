/********************************************
*
*	Programa ejemplo, inicialización del dispositivo háptico
*   y creación de mundo virtual usando cMesh (triángulos)
*	Taller de aplicaciones con hápticos 
*	usando chai3d
*
*   Autor: Eusebio Ricárdez Vázquez
*********************************************/

#include "stdafx.h"
#include "chai3d.h"

//------------------------------------------------------------
//Definición de variables globales
// El mundo que contendrá todo
cWorld* world;

// La camara a través de la cual observaremos el mundo
cCamera* camera;

// Fuente de luz
cLight *light;

cTexture2D* texture ;
// Ancho y alto de la pantalla gráfica
int displayW  = 0;
int displayH  = 0;

// Dimensiones que tendrá la pantalla gráfica
const int WINDOW_SIZE_W         = 600;
const int WINDOW_SIZE_H         = 600;

// Opciones para el menu del ratón 
const int OPTION_FULLSCREEN     = 1;
const int OPTION_WINDOWDISPLAY  = 2;

// Manejador del dispositivo háptico
cHapticDeviceHandler *handler;

// avatar del dispositivo virtual
cGeneric3dofPointer* tool;

// objeto hecho de mallas
//cShapeSphere* object0;
cMesh* object;

// Variable de control para la simulación
bool simulationRunning = false;

// Variable para detectar el fin de la simulación
bool simulationFinished = false;

//---------------------------------------------------------------------------
// Prototipos de funciones
//---------------------------------------------------------------------------

// función de resize para OpenGL
void resizeWindow(int w, int h);

// Detección de teclas 
void keySelect(unsigned char key, int x, int y);

// Menu para el mouse
void menuSelect(int value);

// Función para terminar las ejecuciones antes de cerrar
void close(void);

// Rutina gráfica principal
void updateGraphics(void);

// Rutina principal háptica
void updateHaptics(void);


int main(int argc, char* argv[])
{
//-----------------------------------------------------------------------
// INICIALIZACION
//-----------------------------------------------------------------------

    printf ("\n");
    printf ("-----------------------------------\n");
    printf ("Programa ejemplo para demostrar uso de CHAID 3D\n");
    printf ("Esfera con propiedades hapticas \n");
	printf ("[x] o [ESC]- termina la applicacion \n");
    printf ("-----------------------------------\n");
	printf ("\n\n");
   
  // Creación y configutación del mindo virtual
    world = new cWorld();

    // Elección del color de fondo
    // el color se selecciona usando componentes (R,G,B) 
    world->setBackgroundColor(0.0, 0.0, 0.0);

    // Creacion y configuración de la cámara
    camera = new cCamera(world);
    world->addChild(camera);

    // posicion y orientación de la camara
    camera->set( cVector3d (3, 0.0, 1.5),    // position (eye)
                 cVector3d (0.0, 0.0, 0.0),    // lookat position (target)
                 cVector3d (0.0, 0.0, 1.0));   // dirección del vector "up" 

    // Planos de corte "near y far" de la cámara
    // lo que esté atras o adelante de estos planos no se visualizará
    camera->setClippingPlanes(0.01, 10.0);

    // Habilitación de transparencia
    camera->enableMultipassTransparency(true);

    // Creación de una fuente de luz
    light = new cLight(world);
    camera->addChild(light);                   // Fijar la luz a la cámara
    light->setEnabled(true);                   // Habilitar fuente de luz
    light->setPos(cVector3d( 2.0, 0.5, 1.0));  // Posición de la fuente de luz
    light->setDir(cVector3d(-2.0, 0.5, 1.0));  // Dirección del haz de luz

// Configuración del dispositivo háptico y la esfera de exploración háptica

	// Activación del controlador del dispositivo háptico
	handler = new cHapticDeviceHandler();
	// Accede al primer dispositivo disponible
	cGenericHapticDevice* hapticDevice;
	if(handler->getDevice(hapticDevice, 0) < 0)
	{
		printf("Error: no hay dispositivo disponible");
		exit(1);
	}
	// Obtener la información del dispositivo conectado
	cHapticDeviceInfo info;
	if (hapticDevice != NULL)
	{
		hapticDevice->open();
		hapticDevice->initialize();
		info = hapticDevice->getSpecifications();
	}

  // creación y configuración del cursor háptico 
    tool = new cGeneric3dofPointer(world);
    world->addChild(tool);

    // conecta el dispositivo al cursor háptico
    tool->setHapticDevice(hapticDevice);

    // inicialización del cursor háptico
    tool->start();

    // configura el dispositivo para recorrer el mundo
    tool->setWorkspaceRadius(1.0);
	
    // radio del cursor háptico
    tool->setRadius(0.03);
	// oculta el cursor real (HIP)
	tool->m_deviceSphere->setShowEnabled(false);
	//tool->m_proxySphere->setShowEnabled(true);
	double proxyRadius = 0.03;
    tool->m_proxyPointForceModel->setProxyRadius(proxyRadius);
	
	// Respuesta háptica en ambos lados 
    tool->m_proxyPointForceModel->m_collisionSettings.m_checkBothSidesOfTriangles = true;

	
	// Establece la escala entre el mundo virtual y la capacidad que puede 
	// manejar el háptico
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    
	// define la fuerza máxima que puede manejar el dispositivo háptico actual
	// este valor se combina con la capacidad calculada en el paso anterior
    double stiffnessMax = info.m_maxForceStiffness / workspaceScaleFactor;
    double forceMax = info.m_maxForce;

	// define el valor máximo de amortiguamiento que puede manejar el dispositivo 
	// haptico actual, combinado con el factor de escala.
    
    double dampingMax = info.m_maxLinearDamping / workspaceScaleFactor; 

//-----------------------------------------------------------------------
// Escena virtual
//-----------------------------------------------------------------------


	  // creacion de un objeto virtual
    object = new cMesh(world);

    // agregar el objeto al mundo
    world->addChild(object);

    // posicion del objeto en el centro del mundo
    object->setPos(0.0, 0.0, 0.0);
	
	// vertices para la figura
	int vertices[6][4];
	const double HALFSIZE = 0.5;

    // cara -x
    vertices[0][0] = object->newVertex(-HALFSIZE,  HALFSIZE, -HALFSIZE);
    vertices[0][1] = object->newVertex(-HALFSIZE, -HALFSIZE, -HALFSIZE);
    vertices[0][2] = object->newVertex(-HALFSIZE, -HALFSIZE,  HALFSIZE);
    vertices[0][3] = object->newVertex(-HALFSIZE,  HALFSIZE,  HALFSIZE);

    // cara +x
    vertices[1][0] = object->newVertex( HALFSIZE, -HALFSIZE, -HALFSIZE);
    vertices[1][1] = object->newVertex( HALFSIZE,  HALFSIZE, -HALFSIZE);
    vertices[1][2] = object->newVertex( HALFSIZE,  HALFSIZE,  HALFSIZE);
    vertices[1][3] = object->newVertex( HALFSIZE, -HALFSIZE,  HALFSIZE);

    // cara -y
    vertices[2][0] = object->newVertex(-HALFSIZE,  -HALFSIZE, -HALFSIZE);
    vertices[2][1] = object->newVertex( HALFSIZE,  -HALFSIZE, -HALFSIZE);
    vertices[2][2] = object->newVertex( HALFSIZE,  -HALFSIZE,  HALFSIZE);
    vertices[2][3] = object->newVertex(-HALFSIZE,  -HALFSIZE,  HALFSIZE);

    // cara +y
    vertices[3][0] = object->newVertex( HALFSIZE,   HALFSIZE, -HALFSIZE);
    vertices[3][1] = object->newVertex(-HALFSIZE,   HALFSIZE, -HALFSIZE);
    vertices[3][2] = object->newVertex(-HALFSIZE,   HALFSIZE,  HALFSIZE);
    vertices[3][3] = object->newVertex( HALFSIZE,   HALFSIZE,  HALFSIZE);

    // cara -z
    vertices[4][0] = object->newVertex(-HALFSIZE,  -HALFSIZE, -HALFSIZE);
    vertices[4][1] = object->newVertex(-HALFSIZE,   HALFSIZE, -HALFSIZE);
    vertices[4][2] = object->newVertex( HALFSIZE,   HALFSIZE, -HALFSIZE);
    vertices[4][3] = object->newVertex( HALFSIZE,  -HALFSIZE, -HALFSIZE);

    // cara +z
    vertices[5][0] = object->newVertex( HALFSIZE,  -HALFSIZE,  HALFSIZE);
    vertices[5][1] = object->newVertex( HALFSIZE,   HALFSIZE,  HALFSIZE);
    vertices[5][2] = object->newVertex(-HALFSIZE,   HALFSIZE,  HALFSIZE);
    vertices[5][3] = object->newVertex(-HALFSIZE,  -HALFSIZE,  HALFSIZE);

  // creación de los triángulos
    for (int i=0; i<6; i++)
    {
        object->newTriangle(vertices[i][0], vertices[i][1], vertices[i][2]);
        object->newTriangle(vertices[i][0], vertices[i][2], vertices[i][3]);
    }

	object->rotate(cVector3d(0,0,1),cDegToRad(30));
	// creación de textura
    texture = new cTexture2D();
    object->setTexture(texture);
	// Textura deshabilitada 
    object->setUseTexture(false);

	bool fileload;
	fileload = object->m_texture->loadFromFile("../imagenes/nCedfence.bmp");

	if (!fileload)
	{
		printf("Error - No se pudo cargar la textura.\n");
		close();
		return (-1);
	}
	object->m_texture->setSphericalMappingEnabled(true);


    // cálculo de normales
    object->computeAllNormals();
	
    // cálculo de límites para colisión "Boundary box"
    object->computeBoundaryBox(true);
	// modo wire activado
	object->setWireMode(true);
    
    // Agregar algorítmo de colisiones
    object->createAABBCollisionDetector(1.01 * proxyRadius, true, false);

    // Propiedades hápticas
	// Dureza
    object->setStiffness(stiffnessMax, true);

    // Fricción
    object->setFriction(0.5, 0.3, true);

	
	 //-----------------------------------------------------------------------
    // OPEN GL
    //-----------------------------------------------------------------------

    // inicializacion de  GLUT
    glutInit(&argc, argv);

    // obtención de la resolución actual de la pantala y 
	// ubicacion del gráfico en el centro de la misma
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
    glutSetWindowTitle("Ejemplo - Cubo");    // titulo de la ventana OpenGL

    // Creación de menu para el mouse
    glutCreateMenu(menuSelect);
    glutAddMenuEntry("Pantalla completa", OPTION_FULLSCREEN);
    glutAddMenuEntry("Ventana", OPTION_WINDOWDISPLAY);
    glutAttachMenu(GLUT_RIGHT_BUTTON);


    //-----------------------------------------------------------------------
    // Iniciar simulación
    //-----------------------------------------------------------------------

    // inicio de la simulación
    simulationRunning = true;

    // Creación del hilo para la parte háptica
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
    // Redibuja la ventana gráfica con el nuevo tamaño
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

        // tamaño original
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

    // espera 100 ms para asegurarse que el hilo haptico se cerró
    while (!simulationFinished) { cSleepMs(100); }

    // detener el dispositivo haptico
    tool->stop();
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
    while(simulationRunning)
    {
        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updatePose();

        // compute interaction forces
        tool->computeInteractionForces();

        // send forces to device
        tool->applyForces();
    }
    
    // exit haptics thread
    simulationFinished = true;
}

//---------------------------------------------------------------------------
