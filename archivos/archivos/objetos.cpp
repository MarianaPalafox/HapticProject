/********************************************
*
*	Programa ejemplo, inicializaci�n del dispositivo h�ptico
*   y creaci�n de mundo virtual usando cMesh (tri�ngulos)
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

cTexture2D* texture ;
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

// avatar del dispositivo virtual
cGeneric3dofPointer* tool;

// Objetos hecho de mallas
cMesh* object;
cMesh* object1;
cMesh* object2;
cMesh* object3;

// Variable de control para la simulaci�n
bool simulationRunning = false;

// Variable para detectar el fin de la simulaci�n
bool simulationFinished = false;

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
   
  // Creaci�n y configutaci�n del mindo virtual
    world = new cWorld();

    // Elecci�n del color de fondo
    // el color se selecciona usando componentes (R,G,B) 
    world->setBackgroundColor(0.0, 0.5, 0.5);

    // Creacion y configuraci�n de la c�mara
    camera = new cCamera(world);
    world->addChild(camera);

    // posicion y orientaci�n de la camara
    camera->set( cVector3d (5, 0.0, 1.5),    // position (eye)
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
    light->setPos(cVector3d( 2.0, 0.5, 1.0));  // Posici�n de la fuente de luz
    light->setDir(cVector3d(-2.0, 0.5, 1.0));  // Direcci�n del haz de luz

// Configuraci�n del dispositivo h�ptico y la esfera de exploraci�n h�ptica

	// Activaci�n del controlador del dispositivo h�ptico
	handler = new cHapticDeviceHandler();
	// Accede al primer dispositivo disponible
	cGenericHapticDevice* hapticDevice;
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

  // creaci�n y configuraci�n del cursor h�ptico 
    tool = new cGeneric3dofPointer(world);
    world->addChild(tool);

    // conecta el dispositivo al cursor h�ptico
    tool->setHapticDevice(hapticDevice);

    // inicializaci�n del cursor h�ptico
    tool->start();

    // configura el dispositivo para recorrer el mundo
    tool->setWorkspaceRadius(2.0);
	
    // radio del cursor h�ptico
    tool->setRadius(0.02);
	// oculta el cursor real (HIP)
	tool->m_deviceSphere->setShowEnabled(false);
	double proxyRadius = 0.02;
    tool->m_proxyPointForceModel->setProxyRadius(proxyRadius);
	
	// Respuesta h�ptica en ambos lados 
    tool->m_proxyPointForceModel->m_collisionSettings.m_checkBothSidesOfTriangles = false;

	// ajust the color of the tool
    tool->m_materialProxy = tool->m_materialProxyButtonPressed;
	
	// Establece la escala entre el mundo virtual y la capacidad que puede 
	// manejar el h�ptico
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    
	// define la fuerza m�xima que puede manejar el dispositivo h�ptico actual
	// este valor se combina con la capacidad calculada en el paso anterior
    double stiffnessMax = info.m_maxForceStiffness / workspaceScaleFactor;
    double forceMax = info.m_maxForce;

	// define el valor m�ximo de amortiguamiento que puede manejar el dispositivo 
	// haptico actual, combinado con el factor de escala.
    
    double dampingMax = info.m_maxLinearDamping / workspaceScaleFactor; 

//-----------------------------------------------------------------------
// Escena virtual
//-----------------------------------------------------------------------


	  object = new cMesh(world);

    // agregar el objeto al mundo
    world->addChild(object);

    // posici�n del objeto en el mundo
    object->setPos(0.0, 0.0, -2.0);
	
	// vertices para la figura
	int vertices[1][4];

    vertices[0][0] = object->newVertex(2,-2,2);
    vertices[0][1] = object->newVertex(2,2,2);
    vertices[0][2] = object->newVertex(-2,2,2);
    vertices[0][3] = object->newVertex(-2,-2,2);


  // creaci�n de los tri�ngulos
    object->newTriangle(vertices[0][0], vertices[0][1], vertices[0][2]);
    object->newTriangle(vertices[0][0], vertices[0][2], vertices[0][3]);

	// creaci�n de textura
	
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
    // c�lculo de normales
    object->computeAllNormals();
	
    // c�lculo de l�mites para colisi�n "Boundary box"
    object->computeBoundaryBox(true);
	// modo wire activado
	//object->setWireMode(true);
    
    // Agregar algor�tmo de colisiones
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

    // // Propiedades h�pticas y gr�ficas 
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
    // posici�n del objeto en el mundo
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
    // posici�n del objeto en el mundo
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
	// posici�n del objeto en el mundo
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