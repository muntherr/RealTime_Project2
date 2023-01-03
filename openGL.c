#include "local.h"

char tellers[8][5]={" "," "," "," "," "," "," "," "};
char *shmptr;
struct MEMORY *memptr;
int shmid;
int semid;
char str_menOutside[5];
char str_forked_people_number[5];
char str_people_in_male_gate_counter[5];
char str_people_in_female_gate_counter[5];
char str_groupingAreaCount[5];
char str_terminationVar0[5];
char str_terminationVar1[5];
char str_terminationVar2[5];

void display();
void reshape(int w, int h);
void timer(int);
void reset(int sig);
void drawString18(float x, float y, char *string);
void drawString12(float x, float y, char *string);
void printTellers();
void printMenOutside();
void getMemory();

int main(int argc, char **argv){
    getMemory();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("OIM_Office");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
}

void display(){
    glClearColor(0.161, 0.161, 0.161, 0); //(Red,Green,Blue,Alpha)
    // Set every pixel in the frame buffer to the current clear color.
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    //-------people who left-------//
    drawString12(150, 220, "People who left:");
    drawString18(185, 220, str_terminationVar0);   

    //-------people who's satisfied-------//
    drawString12(150, 210, "people who's satisfied:");
    drawString18(185, 210, str_terminationVar2);   

    //-------people who's unsatisfied-------//
    drawString12(150, 200, "people who's unsatisfied:");
    drawString18(185, 200, str_terminationVar1);   

    //-------outside women-------//
    drawString18(10, 110, "People Outside");
    drawString18(10, 100, str_forked_people_number);

    // //-------outside men-------//
    // drawString18(10, 155, "Men Outside");
    // drawString18(10, 150, str_menOutside);

    //------Office----------//
    glLineWidth(50);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(100, 190);
    glVertex2i(190, 190);
    glVertex2i(190, 10);
    glVertex2i(100, 10);
    glEnd();
    drawString12(110, 110,"People inside the grouping area:");
    drawString18(120, 100, str_groupingAreaCount);

    //------Male Gate----------//
    glLineWidth(30);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(50, 170);
    glVertex2i(100, 170);
    glVertex2i(100, 130);
    glVertex2i(50, 130);
    glEnd();
    if(memptr->male_gate_status>0)
        glColor3f(0,1, 0);
    else
        glColor3f(1, 0, 0);
    glBegin(GL_QUADS);
    glVertex2i(50, 170);
    glVertex2i(60, 170);
    glVertex2i(60, 130);
    glVertex2i(50, 130);
    glEnd();
    drawString12(65, 160, "Men count inside the gate:");
    drawString18(70, 150, str_people_in_male_gate_counter);

    //------Female Gate----------//
    glLineWidth(30);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(50, 70);
    glVertex2i(100, 70);
    glVertex2i(100, 30);
    glVertex2i(50, 30);
    glEnd();
        if(memptr->female_gate_status>0)
        glColor3f(0,1, 0);
    else
        glColor3f(1, 0, 0);
    glBegin(GL_QUADS);
    glVertex2i(50, 70);
    glVertex2i(60, 70);
    glVertex2i(60, 30);
    glVertex2i(50, 30);
    glEnd();
    drawString12(63, 60, "Women count inside the gate:");
    drawString18(70, 50, str_people_in_female_gate_counter);

    //------Male Metal Detector----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(120, 160);
    glVertex2i(130, 160);
    glVertex2i(130, 140);
    glVertex2i(120, 140);
    glEnd();
    if(memptr->m_machine_available>0)
        glColor3f(0, 1, 0);
    else
        glColor3f(1, 0, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f( 120,160);
    glVertex2f( 130, 160);
    glVertex2f( 125,165 );
    glEnd();

    //------Female Metal Detector----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(120, 60);
    glVertex2i(130, 60);
    glVertex2i(130, 40);
    glVertex2i(120, 40);
    glEnd();
    if(memptr->f_machine_available>0)
        glColor3f(0,1, 0);
    else
        glColor3f(1, 0, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f( 120,60);
    glVertex2f( 130, 60);
    glVertex2f( 125,65 );
    glEnd();

    //------Teller 1----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 30);
    glVertex2i(180, 30);
    glVertex2i(180, 20);
    glVertex2i(160, 20);
    glEnd();
    drawString18(165, 25, tellers[0]);

    //------Teller 2----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 50);
    glVertex2i(180, 50);
    glVertex2i(180, 40);
    glVertex2i(160, 40);
    glEnd();
    drawString18(165, 45, tellers[1]);

    //------Teller 3----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 70);
    glVertex2i(180, 70);
    glVertex2i(180, 60);
    glVertex2i(160, 60);
    glEnd();
    drawString18(165, 65, tellers[2]);

    //------Teller 4----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 90);
    glVertex2i(180, 90);
    glVertex2i(180, 80);
    glVertex2i(160, 80);
    glEnd();
    drawString18(165, 85, tellers[3]);

    //------Teller 5----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 120);
    glVertex2i(180, 120);
    glVertex2i(180, 110);
    glVertex2i(160, 110);
    glEnd();
    drawString18(165, 115, tellers[4]);

    //------Teller 6----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 140);
    glVertex2i(180, 140);
    glVertex2i(180, 130);
    glVertex2i(160, 130);
    glEnd();
    drawString18(165, 135, tellers[5]);

    //------Teller 7----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 160);
    glVertex2i(180, 160);
    glVertex2i(180, 150);
    glVertex2i(160, 150);
    glEnd();
    drawString18(165, 155, tellers[6]);
    //------Teller 7----------//
    glLineWidth(0.1);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(160, 180);
    glVertex2i(180, 180);
    glVertex2i(180, 170);
    glVertex2i(160, 170);
    glEnd();
    drawString18(165, 175, tellers[7]);


    glFlush();
}

void reshape(int w, int h){
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 200, 0, 230);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int i){

    glutPostRedisplay();
    glutTimerFunc(1000 / 60, timer, 0);
    printTellers();
    sprintf(str_forked_people_number,"%d",memptr->forked_people_number);
    sprintf(str_people_in_male_gate_counter,"%d",memptr->people_in_male_gate_counter);
    sprintf(str_people_in_female_gate_counter,"%d",memptr->people_in_female_gate_counter);
    sprintf(str_groupingAreaCount,"%d",memptr->groupingAreaCount); 
    sprintf(str_terminationVar0,"%d",memptr->termination_variables[0]);  
    sprintf(str_terminationVar1,"%d",memptr->termination_variables[1]);
    sprintf(str_terminationVar2,"%d",memptr->termination_variables[2]);

}

void drawString18(float x, float y, char *string){
    glRasterPos2f(x, y);

    for (char *c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); // Updates the position
    }
}
void drawString12(float x, float y, char *string){
    glRasterPos2f(x, y);

    for (char *c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); // Updates the position
    }
}

void getMemory(){
    if((shmid = shmget((int)getppid(), 0, 0)) != -1){
        if((shmptr = (char *)shmat(shmid, (char *)0, 0)) == (char *)-1){
            perror("shmat OPEN GL -- producer -- attach");
            exit(1);
        }
        memptr = (struct MEMORY *)shmptr;
    }
    else{
        perror("shmget OPEN GL-- producer -- access");
        exit(2);
    }
}

void printTellers(){

    for(int i=0; i<8; i++){
        char str[5];
        char Nx[5];
        char T[1]; 
        T[0]=memptr->tellers[i];
        switch (T[0]){
            case 'R':
                sprintf(Nx,"%d",memptr->r_serial_number_counter);
                break;
            case 'B':
                sprintf(Nx,"%d",memptr->b_serial_number_counter);
                break;
            case 'I':
                sprintf(Nx,"%d",memptr->i_serial_number_counter);
                break;
            case 'T':
                sprintf(Nx,"%d",memptr->t_serial_number_counter);
                break;
            default:
                strcpy(Nx,"");
                break;
        } 
        strcpy(str,T);
        strcat(str,Nx);
        strcpy(tellers[i],str);
        strcpy(str,"");
        strcpy(Nx,"");
        strcpy(T,"");
    }
    
}
