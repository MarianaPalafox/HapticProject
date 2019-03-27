#include "pch.h"
#include <iostream>
#include "chai3d.h"
#include "GEL3D.h"

cWorld* world;

cCamera* camera;

cLight *light;

cTexture2D* texture;

int displayW = 0;
int displayH = 0;

const int WINDOW_SIZE_W = 600;
const int WINDOW_SIZE_H = 600;

const int OPTION_FULLSCREEN = 1;
const int OPTION_WINDOWDISPLAY = 2;

cHapticDeviceHandler *handler;
cGenericHapticDevice* hapticDevice;

cGELWorld* defWorld;
cGELMesh* defObject;
cGELSkeletonNode* nodes[15][35][3];

cShapeSphere* device;
double deviceRadius;

double radius;

double stiffness;

bool simulationRunning = false;

bool simulationFinished = false;

double deviceForceScale;

double workspaceScaleFactor;

double cursorWorkspaceRadius;

cPrecisionClock simClock;

void resizeWindow(int w, int h);

void keySelect(unsigned char key, int x, int y);

void menuSelect(int value);

void close(void);

void updateGraphics(void);

void updateHaptics(void);

cVector3d computeForce(const cVector3d& a_cursor,
	double a_cursorRadius,
	const cVector3d& a_spherePos,
	double a_radius,
	double a_stiffness);


int main(int argc, char* argv[])
{
	printf("\n");
	printf("같같같같같같같같같같같같같같같같같같같같\n");
	printf("Proyecto final de dispositivos hapticos\n");
	printf("Sillon deformable \n");
	printf("같같같같같같같같같같같같같같같같같같같같\n");
	printf("\n\n");

	world = new cWorld();

	world->setBackgroundColor(1, 1, 1);

	camera = new cCamera(world);
	world->addChild(camera);

	camera->set(cVector3d(-3.5, 0.0, 1.5),    
		cVector3d(0.0, 0.0, 0.0),    
		cVector3d(0.0, 0.0, 1.0));   

	camera->setClippingPlanes(0.01, 10.0);

	camera->enableMultipassTransparency(true);

	light = new cLight(world);
	camera->addChild(light);                   
	light->setEnabled(true);                   
	light->setPos(cVector3d(2.0, 0.5, 1.0));  
	light->setDir(cVector3d(-2.0, 0.5, 1.0));  

	handler = new cHapticDeviceHandler();

	if (handler->getDevice(hapticDevice, 0) < 0)
	{
		printf("Error: no hay dispositivo disponible");
		exit(1);
	}

	cHapticDeviceInfo info;
	if (hapticDevice != NULL)
	{
		hapticDevice->open();
		hapticDevice->initialize();
		info = hapticDevice->getSpecifications();
	}

	cursorWorkspaceRadius = 0.5;
	workspaceScaleFactor = cursorWorkspaceRadius / info.m_workspaceRadius;

	deviceForceScale = 0.1 * info.m_maxForce;

	
	deviceRadius = 0.1;   
	device = new cShapeSphere(deviceRadius);
	world->addChild(device);
	device->m_material.m_ambient.set(0.4, 0.4, 0.4, 0.7);
	device->m_material.m_diffuse.set(0.7, 0.7, 0.7, 0.7);
	device->m_material.m_specular.set(1.0, 1.0, 1.0, 0.7);
	device->m_material.setShininess(100);
	stiffness = 40;

	defWorld = new cGELWorld();
	world->addChild(defWorld);
	world->setPos(1.5, 0.0, 0.1);
	world->rotate(cVector3d(0, 1, 0), cDegToRad(30));

	cGELSkeletonNode::default_kDampingPos = 0.1;
	cGELSkeletonNode::default_kDampingRot = 0.1;
	defWorld->m_integrationTime = 0.001;

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

	cMaterial mat;
	mat.m_ambient.set(0.2, 0.2, 0.2);
	mat.m_diffuse.set(0.6, 0.6, 0.6);
	mat.m_ambient.set(1.0, 1.0, 1.0);
	mat.setShininess(100);

	defObject->setMaterial(mat, true);
	defObject->setTransparencyLevel(0, true);
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


	defObject->buildVertices();
	defObject->m_useSkeletonModel = true;


	cGELSkeletonNode::default_radius = 0.03;
	cGELSkeletonNode::default_kDampingPos = 0.4;
	cGELSkeletonNode::default_kDampingRot = 0.1;
	cGELSkeletonNode::default_mass = 0.2;  // [kg]
	cGELSkeletonNode::default_showFrame = true;
	cGELSkeletonNode::default_color.set(1.0, 0.0, 0.0);
	cGELSkeletonNode::default_useGravity = false;
	cGELSkeletonNode::default_gravity.set(0.00, 0.00, -0.20);
	radius = cGELSkeletonNode::default_radius;
 
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
	

	cGELSkeletonLink::default_kSpringElongation = 100.0; 
	cGELSkeletonLink::default_kSpringFlexion = 0.5;   
	cGELSkeletonLink::default_kSpringTorsion = 0.1;   
	cGELSkeletonLink::default_color.set(0.2, 0.2, 1.0);

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

	defObject->connectVerticesToSkeleton(false);

	defObject->m_showSkeletonModel = true;

	defWorld->m_integrationTime = 0.005;

	glutInit(&argc, argv);

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
	glutSetWindowTitle("Proyecto Final");    

	glutCreateMenu(menuSelect);
	glutAddMenuEntry("Pantalla completa", OPTION_FULLSCREEN);
	glutAddMenuEntry("Ventana", OPTION_WINDOWDISPLAY);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	simulationRunning = true;

	cThread* hapticsThread = new cThread();
	hapticsThread->set(updateHaptics, CHAI_THREAD_PRIORITY_HAPTICS);

	glutMainLoop();

	close();
	return (0);

}


void resizeWindow(int w, int h)
{
	displayW = w;
	displayH = h;
	glViewport(0, 0, displayW, displayH);
}


void keySelect(unsigned char key, int x, int y)
{
	if ((key == 27) || (key == 'x'))
	{
		close();
		exit(0);
	}
	if (key == '1')
	{
		defObject->m_showSkeletonModel = true;
	}
	if (key == '2')
	{
		defObject->m_showSkeletonModel = false;
	}
}

void menuSelect(int value)
{
	switch (value)
	{
	case OPTION_FULLSCREEN:
		glutFullScreen();
		break;
	case OPTION_WINDOWDISPLAY:
		glutReshapeWindow(WINDOW_SIZE_W, WINDOW_SIZE_H);
		break;
	}
}


void close(void)
{
	simulationRunning = false;
	while (!simulationFinished) { cSleepMs(100); }
	hapticDevice->close();
}


void updateGraphics(void)
{
	defWorld->updateSkins();
	defObject->computeAllNormals(true);
	camera->renderView(displayW, displayH);


	glutSwapBuffers();
	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR) printf("Error:  %s\n", gluErrorString(err));

	if (simulationRunning)
	{
		glutPostRedisplay();
	}


}


void updateHaptics(void)
{
	simClock.reset();

	while (simulationRunning)
	{
		simClock.stop();

		double timeInterval = simClock.getCurrentTimeSeconds();
		if (timeInterval > 0.001) { timeInterval = 0.001; }

		simClock.reset();
		simClock.start();

		cVector3d pos;
		hapticDevice->getPosition(pos);
		pos.mul(workspaceScaleFactor);
		device->setPos(pos);

		cVector3d force;
		force.zero();

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

		defWorld->updateDynamics(timeInterval);

		force.mul(deviceForceScale);

		hapticDevice->setForce(force);
	}

	simulationFinished = true;
}

cVector3d computeForce(const cVector3d& a_cursor,
	double a_cursorRadius,
	const cVector3d& a_spherePos,
	double a_radius,
	double a_stiffness)
{

	cVector3d force;
	force.zero();
	cVector3d vSphereCursor = a_cursor - a_spherePos;

	if (vSphereCursor.length() < 0.0000001)
	{
		return (force);
	}

	if (vSphereCursor.length() > (a_cursorRadius + a_radius))
	{
		return (force);
	}

	double penetrationDistance = (a_cursorRadius + a_radius) - vSphereCursor.length();
	cVector3d forceDirection = cNormalize(vSphereCursor);
	force = cMul(penetrationDistance * a_stiffness, forceDirection);

	return (force);
}