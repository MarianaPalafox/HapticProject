// FinalHapticos.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "chai3d.h"
#include "GEL3D.h"

//Definici�n de variables globales
// El mundo que contendr� todo
cWorld* world;

// La camara a trav�s de la cual observaremos el mundo
cCamera* camera;

// Fuente de luz
cLight *light;

cTexture2D* texture;
// Ancho y alto de la pantalla gr�fica
int displayW = 0;
int displayH = 0;

// Dimensiones que tendr� la pantalla gr�fica
const int WINDOW_SIZE_W = 600;
const int WINDOW_SIZE_H = 600;

// Opciones para el menu del rat�n 
const int OPTION_FULLSCREEN = 1;
const int OPTION_WINDOWDISPLAY = 2;

// Manejador del dispositivo h�ptico
cHapticDeviceHandler *handler;

// Dispositivo h�ptico
cGenericHapticDevice* hapticDevice;

//---------------------------------------------------------------------------
// GEL 3D
//---------------------------------------------------------------------------

// Mundo deformable
cGELWorld* defWorld;

// Objeto de mallas deformable
cGELMesh* defObject;

// Nodos din�micos
cGELSkeletonNode* nodes[15][35][3];

// Modelo del objeto
cShapeSphere* device;
double deviceRadius;

// Radio del objeto din�mico
double radius;

// Fuerza entre el h�ptico y el objeto deformable
double stiffness;

//--------------------------------------------------------------------

// Variable de control para la simulaci�n
bool simulationRunning = false;

// Variable para detectar el fin de la simulaci�n
bool simulationFinished = false;

// Factor de escala para la fuerza
double deviceForceScale;

// Factor de escala entre el sapacio total y el �rea del dispositivo
double workspaceScaleFactor;

// Radio de trabajo del cursor virtual
double cursorWorkspaceRadius;

// Reloj para la simulacion
cPrecisionClock simClock;

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

// compute forces between tool and environment
cVector3d computeForce(const cVector3d& a_cursor,
	double a_cursorRadius,
	const cVector3d& a_spherePos,
	double a_radius,
	double a_stiffness);


int main(int argc, char* argv[])
{
	//-----------------------------------------------------------------------
	// INICIALIZACION
	//-----------------------------------------------------------------------

	printf("\n");
	printf("����������������������������������������\n");
	printf("Proyecto final de dispositivos hapticos\n");
	printf("Sillon deformable \n");
	printf("����������������������������������������\n");
	printf("\n\n");

	// Creaci�n y configutaci�n del mindo virtual
	world = new cWorld();

	// Elecci�n del color de fondo
	// el color se selecciona usando componentes (R,G,B) 
	//world->setBackgroundColor(148.0 / 255.0, 48.0 / 255.0, 7.0 / 255.0);
	world->setBackgroundColor(1, 1, 1);

	// Creacion y configuraci�n de la c�mara
	camera = new cCamera(world);
	world->addChild(camera);

	// posicion y orientaci�n de la camara
	camera->set(cVector3d(-3.5, 0.0, 1.5),    // position (eye)
		cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
		cVector3d(0.0, 0.0, 1.0));   // direcci�n del vector "up" 

// Planos de corte "near y far" de la c�mara
// lo que est� atras o adelante de estos planos no se visualizar�
	camera->setClippingPlanes(0.01, 10.0);

	// Habilitaci�n de transparencia
	camera->enableMultipassTransparency(true);

	// Creaci�n de una fuente de luz
	light = new cLight(world);
	camera->addChild(light);                   // Fijar la luz a la c�mara
	light->setEnabled(true);                   // Habilitar fuente de luz
	light->setPos(cVector3d(2.0, 0.5, 1.0));  // Posici�n de la fuente de luz
	light->setDir(cVector3d(-2.0, 0.5, 1.0));  // Direcci�n del haz de luz

// Configuraci�n del dispositivo h�ptico y la esfera de exploraci�n h�ptica

	// Activaci�n del controlador del dispositivo h�ptico
	handler = new cHapticDeviceHandler();
	// Accede al primer dispositivo disponible

	if (handler->getDevice(hapticDevice, 0) < 0)
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
	// espacio de trabajo del cursor h�ptico
	cursorWorkspaceRadius = 0.5;

	// confiiguraci�n de la escala de trabajo f�sica y de trabajo del h�ptico
	workspaceScaleFactor = cursorWorkspaceRadius / info.m_workspaceRadius;

	// escala de la fuerza
	deviceForceScale = 0.1 * info.m_maxForce;

	// Esfera que representa al cursor h�ptico
	deviceRadius = 0.1;   // radio del cursor h�ptico
	device = new cShapeSphere(deviceRadius);
	world->addChild(device);
	device->m_material.m_ambient.set(0.4, 0.4, 0.4, 0.7);
	device->m_material.m_diffuse.set(0.7, 0.7, 0.7, 0.7);
	device->m_material.m_specular.set(1.0, 1.0, 1.0, 0.7);
	device->m_material.setShininess(100);
	stiffness = 40;

	//-----------------------------------------------------------------------
	// Escena virtual
	//-----------------------------------------------------------------------


		 // Crear el mundo que soporta ambiente deformable
	defWorld = new cGELWorld();
	world->addChild(defWorld);
	world->setPos(1.5, 0.0, 0.1);
	world->rotate(cVector3d(0, 1, 0), cDegToRad(30));

	// Ajustar estos valores cuidadosamente
	cGELSkeletonNode::default_kDampingPos = 0.1;
	cGELSkeletonNode::default_kDampingRot = 0.1;
	defWorld->m_integrationTime = 0.001;

	// Crear la malla deformable
	defObject = new cGELMesh(world);
	defWorld->m_gelMeshes.push_front(defObject);
	bool fileload;
	fileload = defObject->loadFromFile("../imagenes/Fridhem_3ds.3DS");
	if (!fileload)
	{
		printf("Error al cargar el modelo 3d.\n");
		close();
		return (-1);
	}

	// Configurar los materiales del objeto 
	cMaterial mat;
	mat.m_ambient.set(0.2, 0.2, 0.2);
	mat.m_diffuse.set(0.6, 0.6, 0.6);
	mat.m_ambient.set(1.0, 1.0, 1.0);
	mat.setShininess(100);

	defObject->setMaterial(mat, true);
	defObject->setTransparencyLevel(.6, true);
	defObject->setUseTexture(true, true);
	defObject->scale(0.28);

	cTexture2D* texture = new cTexture2D();
	fileload = texture->loadFromFile("../imagenes/red_sofa.bmp");
	if (!fileload)
	{
		printf("Error al cargar el archivo de textura.\n");
		close();
		return (-1);
	}

	texture->setEnvironmentMode(GL_DECAL);
	texture->setSphericalMappingEnabled(true);

	defObject->setTexture(texture, true);
	defObject->setTransparencyLevel(0.7, true);
	defObject->setUseTexture(true, true);


	// Construir los v�rtices din�micos
	defObject->buildVertices();
	defObject->m_useSkeletonModel = true;


	// Valores default de los nodos
	//cGELSkeletonNode::default_radius        = 0.03;
	cGELSkeletonNode::default_radius = 0.02;
	cGELSkeletonNode::default_kDampingPos = 0.4;
	cGELSkeletonNode::default_kDampingRot = 0.1;
	cGELSkeletonNode::default_mass = 0.2;  // [kg]
	cGELSkeletonNode::default_showFrame = true;
	cGELSkeletonNode::default_color.set(1.0, 0.0, 0.0);
	cGELSkeletonNode::default_useGravity = false;
	// cGELSkeletonNode::default_useGravity    = false;
	cGELSkeletonNode::default_gravity.set(0.00, 0.00, -0.20);
	radius = cGELSkeletonNode::default_radius;

	// Arreglo de nodos 
	for (int y = 0; y < 35; y++)
	{
		for (int x = 0; x < 15; x++)
		{
			for (int z = 0; z < 3; z++) {
				cGELSkeletonNode* newNode = new cGELSkeletonNode();
				nodes[x][y][z] = newNode;
				defObject->m_nodes.push_front(newNode);
				newNode->m_pos.set((0 + 0.05*(double)x), (0 + 0.05*(double)y), (0.35 + 0.05*(double)z));
			}
		}
	}

	// Fijar los nodos de las cuatro esquinas
	nodes[0][0][0]->m_fixed = true;
	nodes[0][34][0]->m_fixed = true;
	nodes[14][0][0]->m_fixed = true;
	nodes[14][34][0]->m_fixed = true;
	nodes[0][0][1]->m_fixed = true;
	nodes[0][34][1]->m_fixed = true;
	nodes[14][0][1]->m_fixed = true;
	nodes[14][34][1]->m_fixed = true;
	nodes[0][0][2]->m_fixed = true;
	nodes[0][34][2]->m_fixed = true;
	nodes[14][0][2]->m_fixed = true;
	nodes[14][34][2]->m_fixed = true;
	

	// Valores default para los enlaces 
	cGELSkeletonLink::default_kSpringElongation = 100.0; // [N/m]
	cGELSkeletonLink::default_kSpringFlexion = 0.5;   // [Nm/RAD]
	cGELSkeletonLink::default_kSpringTorsion = 0.1;   // [Nm/RAD]
	cGELSkeletonLink::default_color.set(0.2, 0.2, 1.0);

	// crear los enlaces entre los nodos 
	for (int y = 0; y < 34; y++) {
		for (int x = 0; x < 14; x++) {
			for (int z = 0; z < 2; z++) {
				cGELSkeletonLink* newLinkX0 = new cGELSkeletonLink(nodes[x + 0][y + 0][z], nodes[x + 1][y + 0][z]);
				cGELSkeletonLink* newLinkX1 = new cGELSkeletonLink(nodes[x + 0][y + 1][z], nodes[x + 1][y + 1][z]);
				cGELSkeletonLink* newLinkY0 = new cGELSkeletonLink(nodes[x + 0][y + 0][z], nodes[x + 0][y + 1][z]);
				cGELSkeletonLink* newLinkY1 = new cGELSkeletonLink(nodes[x + 1][y + 0][z], nodes[x + 1][y + 1][z]);
				defObject->m_links.push_front(newLinkX0);
				defObject->m_links.push_front(newLinkX1);
				defObject->m_links.push_front(newLinkY0);
				defObject->m_links.push_front(newLinkY1);
			}
		}
	}

	for (int y = 0; y < 34; y++) {
		for (int x = 0; x < 14; x++) {
			cGELSkeletonLink * link0and1 = new cGELSkeletonLink(nodes[x][y][0], nodes[x][y][1]);
			cGELSkeletonLink * link1and2 = new cGELSkeletonLink(nodes[x][y][0], nodes[x][y][1]);
			defObject->m_links.push_front(link0and1);
			defObject->m_links.push_front(link1and2);
		}
	}

	// conectar la malla al esqueleto (GEM)
	defObject->connectVerticesToSkeleton(false);

	// Mostrar ocultar el esqueleto
	defObject->m_showSkeletonModel = true;

	// definir la constante de integraci�n
	defWorld->m_integrationTime = 0.005;


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
	glutSetWindowTitle("Ejemplo bjeto deformable");    // titulo de la ventana OpenGL

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


	// opcion 1:
	if (key == '1')
	{
		// muestra esqueleto
		defObject->m_showSkeletonModel = true;
		//defObject->setWireMode(true);
	}

	// opcion 2:
	if (key == '2')
	{
		// oculta esqueleto
		defObject->m_showSkeletonModel = false;
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

	// actualiza la malla del mundo deformable
	defWorld->updateSkins();
	defObject->computeAllNormals(true);

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
	// inicializa reloj
	simClock.reset();

	// loop principal del h�ptico
	while (simulationRunning)
	{
		// detener reloj de simulacion
		simClock.stop();

		// leer el intervalo de tiempo den segundos
		double timeInterval = simClock.getCurrentTimeSeconds();
		if (timeInterval > 0.001) { timeInterval = 0.001; }

		// reinicia el reloj de simulaci�n
		simClock.reset();
		simClock.start();

		// lee la posici�n del dispositivo 
		cVector3d pos;
		hapticDevice->getPosition(pos);
		pos.mul(workspaceScaleFactor);
		device->setPos(pos);

		// C�lculo de fuerza en una variable temporal
		cVector3d force;
		force.zero();

		// calcula las fuerzas de reacci�n del objeto deformable
		for (int y = 0; y < 34; y++) {
			for (int x = 0; x < 14; x++) {
				for (int z = 0; z < 2; z++) {
					cVector3d nodePos = nodes[x][y][z]->m_pos;
					cVector3d f = computeForce(pos, deviceRadius, nodePos, radius, stiffness);
					cVector3d tmpfrc = cNegate(f);
					nodes[x][y][z]->setExternalForce(tmpfrc);
					force.add(f);
				}
			}
		}

		// Actualiza el mundo din�mico
		defWorld->updateDynamics(timeInterval);

		// escala la fuerza
		force.mul(deviceForceScale);

		// Env�a la fuerza al dispositivo h�ptico
		hapticDevice->setForce(force);
	}

	// fin del loop h�ptico
	simulationFinished = true;
}

//---------------------------------------------------------------------------


cVector3d computeForce(const cVector3d& a_cursor,
	double a_cursorRadius,
	const cVector3d& a_spherePos,
	double a_radius,
	double a_stiffness)
{

	// Calcula la fuerza de reacci�n entre el cursor (haptico) y cada una de las esferas
	cVector3d force;
	force.zero();
	cVector3d vSphereCursor = a_cursor - a_spherePos;

	// Verifica si hay colision
	if (vSphereCursor.length() < 0.0000001)
	{
		return (force);
	}

	if (vSphereCursor.length() > (a_cursorRadius + a_radius))
	{
		return (force);
	}

	// Calcula la distancia de penetraci�n entre el cursor y la superficie de la esfera
	double penetrationDistance = (a_cursorRadius + a_radius) - vSphereCursor.length();
	cVector3d forceDirection = cNormalize(vSphereCursor);
	force = cMul(penetrationDistance * a_stiffness, forceDirection);

	// regresa la fuerza 
	return (force);
}