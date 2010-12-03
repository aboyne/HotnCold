#include <iostream>
#include "gridartist.h"

#include <GL/glut.h>    // Header File For The GLUT Library 
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include "SDL.h"
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include <unistd.h>     // needed to sleep.
#include "SDL_audio.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>   
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;

SDL_Surface *surface;
int** thegrid;
GridArtist * myartist;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 16;


GLfloat xrot; /* X Rotation ( NEW ) */
GLfloat yrot; /* Y Rotation ( NEW ) */
GLfloat zrot; /* Y Rotation ( NEW ) */
GLfloat zoom;

struct sockaddr_in sAddr, cAddr;  //connector's address information
int sock;
unsigned char buf[1250];
unsigned char tbuf[1250];



int sendMessage(char *msg, int msg_size)
{
	int res = sendto(sock, msg, msg_size, 0,(struct sockaddr *) &sAddr, sizeof(struct sockaddr_in));
	cout<<"Sent: "<<msg<<" "<<res<<endl;	
	return res;
}

unsigned char* getMessage()
{
	socklen_t fromlen;

	//cout<<"Message: ";
	
	//Receive meesage, if there is one available
	int res = recv(sock, tbuf, 1250,MSG_DONTWAIT);
	//cout<<res<<endl;
	
	//if message valid message recieved, update the map buffer	
	if(res>-1){
		for(int i=0;i<1250;i++){
			buf[i] = tbuf[i];
		}
		//cout<<"Got Message:"<<buf<<endl;
	}
	return buf;

}


int serv_init(char* server, int sport, int cport)
{
	sock = socket(AF_INET,SOCK_DGRAM,0);

	//Set up the IP header for sending to the server
	struct hostent *he = gethostbyname(server);

    	sAddr.sin_family = AF_INET;     // host byte order
   	sAddr.sin_port = htons(sport); // short, network byte order
   	memset(&(sAddr.sin_zero), '\0', 8);  // zero the rest of the struct
   	sAddr.sin_addr = *((struct in_addr *)he->h_addr);


    	cAddr.sin_family = AF_INET;     // host byte order
   	cAddr.sin_port = htons(cport); // short, network byte order
   	memset(&(sAddr.sin_zero), '\0', 8);  // zero the rest of the struct
	bind(sock,(struct sockaddr *) &cAddr, sizeof(struct sockaddr_in));

	//send a message letting them know we are a new client
	char msg[10];
	sprintf(msg,"%d",cport);
	sendMessage(msg, 6);
	
	return 0;
}



void handleKeyPress( SDL_keysym *keysym )
{
	//send a move command to the server.

    switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
	    exit(0);
	    break;
	case SDLK_F1:
	    SDL_WM_ToggleFullScreen( surface );
	    break;
	case SDLK_p:
		break;
	case SDLK_UP:
		if (zoom <= -12) zoom += 15;
		break;
	case SDLK_DOWN:
		zoom -= 15;
		break;
	case SDLK_RIGHT:
		zrot += 15;
		break;
	case SDLK_LEFT:
		zrot -= 15;
		break;
	case SDLK_w:
		sendMessage((char *)"w",1);
		break;
	case SDLK_a:
		sendMessage((char *)"a",1);
		break;
	case SDLK_s:
		sendMessage((char *)"s",1);
		break;
	case SDLK_d:
		sendMessage((char *)"d",1);
		break;
	case SDLK_u:
		sendMessage((char *)"u",1);
		break;
	case SDLK_i:
		sendMessage((char *)"i",1);
		break;
	case SDLK_o:
		sendMessage((char *)"o",1);
		break;
	case SDLK_j:
		sendMessage((char *)"j",1);
		break;
	case SDLK_k:
		sendMessage((char *)"k",1);
		break;
	case SDLK_l:
		sendMessage((char *)"l",1);
		break;
	default:
	    break;
	}

    return;
}

void DecryptAndSetArray(unsigned char* buffer)
{
	for(int i = 0; i < GRID_SIZE * GRID_SIZE ; i++)
	{
		//cout << buffer[i] << " : " <<(int)buffer[i] << endl;
		int j = i % GRID_SIZE;
		int tempi = i / GRID_SIZE;
		thegrid[tempi][j] = ((int)(unsigned char)buffer[i])- 127;
	}
}

int drawGLScene()
{
	static GLint T0 = 0;
	static GLint Frames = 0;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();
		glTranslatef(0.0f,0.0f,-30.0f + zoom);
	
	glRotatef( xrot, 1.0f, 0.0f, 0.0f); //Rotate On The X Axis
	glRotatef( yrot, 0.0f, 1.0f, 0.0f); // Rotate On The Y Axis 
	glRotatef( zrot, 0.0f, 0.0f, 1.0f); // Rotate On The Z Axis

    	myartist->DrawGrid(thegrid);

    	
    	SDL_GL_SwapBuffers( );
    	Frames ++;
	{
		GLint t = SDL_GetTicks();
		if (t - T0 >= 5000)
		{
			GLfloat seconds = (t - T0) / 1000.0;
			GLfloat fps = Frames / seconds;
			printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
			T0 = t;
			Frames = 0;
		}
	}
}



//unchanged
bool resizeWindow(int neww, int newh)
{
	GLfloat ratio;
	if(!newh)newh++;
	ratio = (GLfloat)neww / (GLfloat)newh;
	glViewport(0,0,(GLint)neww,(GLint)newh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective( 45.0f, ratio, 0.1f, 100.0f );
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	return 1;
}

bool initGL()
{

	/* Load in the texture */
	//if ( !LoadGLTextures( ) )
	//	return 0;

	/* Enable Texture Mapping ( NEW ) */
	glEnable( GL_TEXTURE_2D );

	/* Enable smooth shading */
	glShadeModel( GL_SMOOTH );

	/* Set the background black */
	glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );

	/* Depth buffer setup */
	glClearDepth( 1.0f );

	/* Enables Depth Testing */
	glEnable( GL_DEPTH_TEST );

	/* The Type Of Depth Test To Do */
	glDepthFunc( GL_LEQUAL );

	/* Really Nice Perspective Calculations */
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	return 1;
}




int main(int argc, char** argv)
{
	if(argc != 4)
	{
		cout << "bad args" << endl;
		exit(1);
	}
	xrot = (GLfloat)-45;
	yrot = (GLfloat)0;

	char *server = argv[1];
	int sport = atol(argv[2]);
	int cport = atol(argv[3]);
	serv_init(server, sport, cport);
	

	
	thegrid = new int*[GRID_SIZE];
	for(int i = 0; i < GRID_SIZE; i++)
	{
		thegrid[i] = new int[GRID_SIZE];
	}
	
	myartist = new GridArtist();
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			thegrid[i][j] = i * 2 / (j+1);
		}
	}
	
	
	bool done = 0;
	
	
	SDL_Event event;
	const SDL_VideoInfo *videoInfo;//Video Info? not used.
	
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){cerr << "Could Not Initialize Video" << endl; exit(0);}
	videoInfo = SDL_GetVideoInfo();
	if(videoInfo == NULL){cerr << "Empty VideoInfo" << endl; exit(0);}
	int videoFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE | SDL_RESIZABLE;
	if(videoInfo->hw_available)
		videoFlags |= SDL_HWSURFACE;
	else
		videoFlags |= SDL_SWSURFACE;
	if(videoInfo->blit_hw)
		videoFlags |= SDL_HWACCEL;
		
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,videoFlags);
	if(surface == NULL){cerr << "NoSurface" << endl; exit(0);}
	if(!initGL( )){cerr << "Texture shi" << endl; exit(0);}
	resizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_ACTIVEEVENT:
					break;
				case SDL_VIDEORESIZE:
					surface = SDL_SetVideoMode(event.resize.w,event.resize.h,16, videoFlags);
					if(surface == NULL){cerr << "NoSurface" << endl; exit(0);}
					resizeWindow( event.resize.w, event.resize.h );
					break;
				case SDL_KEYDOWN:
					handleKeyPress(&event.key.keysym);
					break;
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}

			//cout << "just before" << endl;

			//cout << buffer << endl;
			
				unsigned char* buffer = getMessage();
				DecryptAndSetArray(buffer);		
				drawGLScene();
				//sleep(1)

	}
}




