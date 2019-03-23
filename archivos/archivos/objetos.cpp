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

// Objetos hecho de mallas
cMesh* object;
cMesh* object1;
cMesh* object2;
cMesh* object3;

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

// cargar modelo
bool loadModel(const char* filename, cMesh* gObject);

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
    world->setBackgroundColor(0.0, 0.5, 0.5);

    // Creacion y configuración de la cámara
    camera = new cCamera(world);
    world->addChild(camera);

    // posicion y orientación de la camara
    camera->set( cVector3d (5, 0.0, 1.5),    // position (eye)
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
    tool->setWorkspaceRadius(2.0);
	
    // radio del cursor háptico
    tool->setRadius(0.02);
	// oculta el cursor real (HIP)
	tool->m_deviceSphere->setShowEnabled(false);
	double proxyRadius = 0.02;
    tool->m_proxyPointForceModel->setProxyRadius(proxyRadius);
	
	// Respuesta háptica en ambos lados 
    tool->m_proxyPointForceModel->m_collisionSettings.m_checkBothSidesOfTriangles = false;

	// ajust the color of the tool
    tool->m_materialProxy = tool->m_materialProxyButtonPressed;
	
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


	  object = new cMesh(world);

    // agregar el objeto al mundo
    world->addChild(object);

    // posición del objeto en el mundo
    object->setPos(0.0, 0.0, -2.0);
	
	// vertices para la figura
	int vertices[1][4];

    vertices[0][0] = object->newVertex(2,-2,2);
    vertices[0][1] = object->newVertex(2,2,2);
    vertices[0][2] = object->newVertex(-2,2,2);
    vertices[0][3] = object->newVertex(-2,-2,2);


  // creación de los triángulos
    object->newTriangle(vertices[0][0], vertices[0][1], vertices[0][2]);
    object->newTriangle(vertices[0][0], vertices[0][2], vertices[0][3]);

	// creación de textura
	
    texture = new cTexture2D();
    object->setTexture(texture);
	
    object->setUseTexture(true);

	bool fileload;
	fileload = object->m_texture->loadFromFile("../imagenes/plastic.bmp");

	if (!fileload)
	{
		printf("Error - No se pudo cargar la textura.\n");
		close();
		return (-1);
	}
	object->m_texture->setWrapMode(GL_REPEAT ,GL_REPEAT );
    // cálculo de normales
    object->computeAllNormals();
	
    // cálculo de límites para colisión "Boundary box"
    object->computeBoundaryBox(true);
	// modo wire activado
	//object->setWireMode(true);
    
    // Agregar algorítmo de colisiones
    object->createAABBCollisionDetector(1.01 * proxyRadius, true, false);

	// coordenadas de textura
    cVertex* v;
    v = object->getVertex(0);
    v->setTexCoord(1,0);
    v = object->getVertex(1);
    v->setTexCoord(1,1);
    v = object->getVertex(2);
    v->setTexCoord(0,1);
    v = object->getVertex(3);
    v->setTexCoord(0,0);
    object->setUseTexture(true);

    // // Propiedades hápticas y gráficas 
    cMaterial matGround;
    matGround.setStiffness(0.4 * stiffnessMax);
    matGround.setDynamicFriction(0.7);
    matGround.setStaticFriction(1.0);
    matGround.m_ambient.set(0.4, 0.4, 0.4);
    matGround.m_diffuse.set(0.5, 0.5, 0.5);
    matGround.m_specular.set(1.0, 1.0, 1.0);
    object->setMaterial(matGround);

   // siguiente objeto: conejo 
	object1 = new cMesh(world);
    // agregar el objeto al mundo
    //world->addChild(object1);
    // posición del objeto en el mundo
    object1->setPos(0.5, 0.0, 0.5);
	fileload = object1->loadFromFile("../imagenes/bunny.obj");
	if (!fileload)
	{
		printf("Error - No se pudo cargar el modelo.\n");
		close();
		return (-1);
	}
	  // calcula  boundary box
    object1->computeBoundaryBox(true);
    // Escala el objeto
    object1->scale(0.3);
    // compute collision detection algorithm
    object1->createAABBCollisionDetector(1.01 * proxyRadius, true, false);
    // define resistencia del objeto
    object1->setStiffness(0.8 * stiffnessMax, true);
	object1->setFriction(1.0,0.5,true);
	object1->rotate(cVector3d(1,0,0),CHAI_PI/2);

	// siguiente objeto: calabaza 
	object2 = new cMesh(world);
    // agregar el objeto al mundo
    world->addChild(object2);
    // posición del objeto en el mundo
    object2->setPos(1.5, -1.0, 0.5);
	fileload = object2->loadFromFile("../imagenes/pumpkinNormalized.obj");
	if (!fileload)
	{
		printf("Error - No se pudo cargar el modelo.\n");
		close();
		return (-1);
	}
	  // compute a boundary box
    object2->computeBoundaryBox(true);
    // Escala el objeto
    object2->scale(0.1);
    // compute collision detection algorithm
    object2->createAABBCollisionDetector(1.01 * proxyRadius, true, false);
    // define resistencia del objeto
    object2->setStiffness(0.8 * stiffnessMax, true);
	object2->rotate(cVector3d(1,0,0),CHAI_PI/2);
	object2->rotate(cVector3d(0,0,1),CHAI_PI/2);


	// siguiente objeto rifle
	object3 = new cMesh(world);
	// posición del objeto en el mundo
	object3->setPos(1.5, 1, 0.5);
	//world->addChild(object3);
    
	fileload = object3->loadFromFile("../imagenes/mp5_sil.3ds");
	if (!fileload)
	{
		printf("Error - No se pudo cargar el modelo.\n");
		close();
		return (-1);
	}
	  

	object3->scale(0.03);
    	
	  // compute a boundary box
    object3->computeBoundaryBox(true);
    // Escala el objeto
	object3->setUseTexture(true);
	object3->rotate(cVector3d(0,0,1),CHAI_PI/2);
	

	// siguiente objeto jeep
    cMesh* object4 = new cMesh(world);

    // add object to world
    //world->addChild(object4);

    // set the position of the object at the center of the world
    object4->setPos(1.3, 0.0, 0.0);
	fileload = object4->loadFromFile("../imagenes/jeep1.3ds");
	if (!fileload)
	{
		printf("Error - No se pudo cargar el modelo: %s\n","jeep");
		close();
		return (-1);
	}
	
 
    object4->rotate( cVector3d(0, 0, 1), cDegToRad(-45));
	object4->scale(0.06);
    
	
   

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
    glutSetWindowTitle("Ejemplo");    // titulo de la ventana OpenGL

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
