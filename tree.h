/*
void draw_branch(GLfloat size, GLfloat length)
{
	GLUquadricObj *pObj; // Quadric object
	pObj =gluNewQuadric();
	gluQuadricNormals(pObj, GLU_SMOOTH);
	
	glPushMatrix();
	gluSphere(pObj, size, 26, 13);
	gluCylinder(pObj, size, size, length, 26, 13);
	glTranslatef(0.0f, 0.0f, length);
	gluSphere(pObj, size, 26, 13);
	glPopMatrix();	
}

void divide_branch_third(int n, GLfloat size, GLfloat length)
{
	if(n){
		if(n<4)
		glColor3ub(107, 79, 65); //brigth dark brown (young branch)
		else glColor3ub(92, 64, 51); //dark brown (branch)

		glPushMatrix();
		draw_branch(size, length); //main branch
		glTranslatef(0.0, 0.0, length);
		glRotatef(45, 0.0, 1.0, 0.0);
		divide_branch_third(n-1, size*0.7, length*0.7); //right branch
		if(n>3){
		glRotatef(-90, 0.0, 1.0, 0.0);
		divide_branch_third(n-1, size*0.5, length*0.5); //left branch
		if(n>2){
		glRotatef(-90, 1.0, 0.0, 1.0);
		divide_branch_third(n-1, size*0.7, length*0.3); //upper branch
		if(n>1){
		glRotatef(90, 0.0, 01.0, 0.0);
		divide_branch_third(n-1, size*0.6, length*0.7); //lower branch
		}}}
		glPopMatrix();
	}
	else{
		glPushMatrix();
		glColor3ub(0, 100, 0); //brigth dark green (leaf)
		glScalef(2.0, 0.5, 1.0);
		draw_branch(0.5, 0.3);
		glPopMatrix();
	}
}
*/

/***************************first tree function*********************************/
void draw_branch(GLfloat size, GLfloat length, GLfloat x, GLfloat y, GLfloat z)
{
	GLUquadricObj *pObj; // Quadric object
	pObj =gluNewQuadric();
	gluQuadricNormals(pObj, GLU_SMOOTH);
	
	glPushMatrix();
	gluSphere(pObj, size, 26, 13);
	gluCylinder(pObj, size, size, length, 26, 13);
	glTranslatef(0.0f, 0.0f, length);
	gluSphere(pObj, size, 26, 13);
	glPopMatrix();	
}

void divide_branch(int n, GLfloat size, GLfloat length, GLfloat x, GLfloat y, GLfloat z)
{
	if(n){
		if(n<4)
		glColor3ub(0, 255, 0); //green (leaf)
		else glColor3ub(92, 51, 23); //sweet brown (branch)
		glPushMatrix();
		draw_branch(size, length, x, y, z); //main branch
		glTranslatef(0.0, 0.0, length);
		glRotatef(45, 0.0, 1.0, 0.0);
		divide_branch(n-1, size*0.7, length*0.7, x, y, z); //right branch
		glRotatef(-90, 0.0, 1.0, 0.0);
		divide_branch(n-1, size*0.7, length*0.7, x, y, z ); //left branch
		glPopMatrix();
	}
	else
		draw_branch(size, length, x, y, z);
}
/******************************end of first tree function**********************************/

/***************************second tree function*********************************/
//use draw_brach from first tree function ;D

void divide_branch_second(int n, GLfloat size, GLfloat length, GLfloat x, GLfloat y, GLfloat z)
{
	if(n){
		if(n<2)
		glColor3ub(0, 100, 0); //brigth dark green (leaf)
		else glColor3ub(92, 64, 51); //dark brown (branch)

		glPushMatrix();
		draw_branch(size, length, x, y, z); //main branch
		glTranslatef(0.0, 0.0, length);
		glRotatef(45, 0.0, 1.0, 0.0);
		divide_branch_second(n-1, size*0.7, length*0.7, x, y, z); //right branch
		glRotatef(-90, 0.0, 1.0, 0.0);
		divide_branch_second(n-1, size*0.7, length*0.7, x, y, z ); //left branch
		glRotatef(-90, 1.0, 0.0, 1.0);
		divide_branch_second(n-1, size*0.7, length*0.7, x, y, z ); //upper branch
		glRotatef(90, 0.0, 01.0, 0.0);
		divide_branch_second(n-1, size*0.7, length*0.7, x, y, z ); //lower branch
		glPopMatrix();
	}
	else{
		glPushMatrix();
		glScalef(2.0, 0.5, 1.0);
		draw_branch(size, length, x, y, z);
		glPopMatrix();
	}
}
/******************************end of second tree function**********************************/
/***************************third tree function*********************************/
//use draw_brach from first tree function ;D

void divide_branch_third(int n, GLfloat size, GLfloat length, GLfloat x, GLfloat y, GLfloat z)
{
	if(n){
		if(n<4)
		glColor3ub(107, 79, 65); //brigth dark brown (young branch)
		else glColor3ub(92, 64, 51); //dark brown (branch)

		glPushMatrix();
		draw_branch(size, length, x, y, z); //main branch
		glTranslatef(0.0, 0.0, length);
		glRotatef(45, 0.0, 1.0, 0.0);
		divide_branch_third(n-1, size*0.7, length*0.7, x, y, z); //right branch
		if(n>3){
		glRotatef(-90, 0.0, 1.0, 0.0);
		divide_branch_third(n-1, size*0.5, length*0.5, x, y, z ); //left branch
		if(n>2){
		glRotatef(-90, 1.0, 0.0, 1.0);
		divide_branch_third(n-1, size*0.7, length*0.3, x, y, z ); //upper branch
		if(n>1){
		glRotatef(90, 0.0, 01.0, 0.0);
		divide_branch_third(n-1, size*0.6, length*0.7, x, y, z ); //lower branch
		}}}
		glPopMatrix();
	}
	else{
		glPushMatrix();
		glColor3ub(0, 100, 0); //brigth dark green (leaf)
		glScalef(2.0, 0.5, 1.0);
		draw_branch(0.5, 0.3, x, y, z);
		glPopMatrix();
	}
}
/******************************end of third tree function**********************************/
/******************************fourth tree function************************************/

void divide_branch_fourth(int n, GLfloat size, GLfloat length, GLfloat x, GLfloat y, GLfloat z)
{
	if(n){
		if(n<2)
		glColor3ub(47, 79, 47); //pine green (leaf)
		else glColor3ub(109, 91, 64); //bright black brown (branch)
		glPushMatrix(); //first
		draw_branch(size, length, x, y, z); //main branch
		glTranslatef(0.0, 0.0, length*0.4);
		glPushMatrix(); //second
		glRotatef(100, 0.0, 1.0, 0.0);
		divide_branch_fourth(n-1, size*0.9, length*0.5, x, y, z); //right branch
		glRotatef(-200, 0.0, 1.0, 0.0);
		divide_branch_fourth(n-1, size*0.9, length*0.5, x, y, z ); //left branch
		glPopMatrix(); //second
		glTranslatef(0.0, 0.0, length*0.2);
		glPushMatrix(); //third
		glRotatef(100, 0.0, 1.0, 0.0);
		divide_branch_fourth(n-1, size*0.9, length*0.3, x, y, z); //right branch
		glRotatef(-200, 0.0, 1.0, 0.0);
		divide_branch_fourth(n-1, size*0.9, length*0.3, x, y, z ); //left branch
		glPopMatrix(); //third
		glTranslatef(0.0, 0.0, length*0.2);
		glPushMatrix(); //fourth
		glRotatef(100, 0.0, 1.0, 0.0);
		divide_branch_fourth(n-1, size*0.9, length*0.15, x, y, z); //right branch
		glRotatef(-200, 0.0, 1.0, 0.0);
		divide_branch_fourth(n-1, size*0.9, length*0.15, x, y, z ); //left branch
		glPopMatrix(); //fourth
		glTranslatef(0.0, 0.0, length*0.1);
		glPushMatrix(); //fifth
		divide_branch_fourth(n-1, size*0.9, length*0.1, x, y, z); //right branch
		glPopMatrix(); //fifth
		glPopMatrix(); //first
	}
	else{
		draw_branch(size, length, x, y, z);
	}
}
/******************************end of fourth tree function**********************************/
/******************************fifth tree function************************************/
void draw_branch_fifth(GLfloat size, GLfloat length, GLfloat x, GLfloat y, GLfloat z)
{
	glPushMatrix();
	glScalef(0.50f, 3.0f, 1.0f);
	draw_branch(size, length, 0.0, 0.0, 0.0);
	glTranslatef(0.0, 0.0, length);
	glRotatef(50, 0.0, 1.0, 0.0);
	draw_branch(size, length*0.7, 0.0, 0.0, 0.0);
	glTranslatef(0.0, 0.0, length*0.7);
	glRotatef(50, 0.0, 1.0, 0.0);
	draw_branch(size*0.5, length*0.5, 0.0, 0.0, 0.0);
	glTranslatef(0.0, 0.0, length*0.5);
	glRotatef(50, 0.0, 1.0, 0.0);
	draw_branch(size*0.3, length*0.3, 0.0, 0.0, 0.0);
	glTranslatef(0.0, 0.0, length*0.3);
	glRotatef(50, 0.0, 1.0, 0.0);
	draw_branch(size*0.3, length*0.1, 0.0, 0.0, 0.0);
		

	glPopMatrix();	
}

void divide_branch_fifth(int n, GLfloat size, GLfloat length, GLfloat x, GLfloat y, GLfloat z)
{

	if(n){
		if(n<2)
		glColor3ub(0, 100, 0); //brigth dark green (leaf)
		else glColor3ub(92, 64, 51); //dark brown (branch)

		glPushMatrix();
		draw_branch_fifth(size, length, x, y, z); //main branch
		glTranslatef(0.0, 0.0, length);
		glRotatef(30, 0.0, 1.0, 0.0);
		divide_branch_fifth(n-1, size*0.7, length*1.5, x, y, z); //right branch
		glRotatef(-60, 0.0, 1.0, 0.0);
		divide_branch_fifth(n-1, size*0.7, length*1.5, x, y, z ); //left branch
		glRotatef(-60, 1.0, 0.0, 1.0);
		divide_branch_fifth(n-1, size*0.7, length*1.5, x, y, z ); //upper branch
		glRotatef(60, 0.0, 01.0, 0.0);
		divide_branch_fifth(n-1, size*0.7, length*1.5, x, y, z ); //lower branch
		glPopMatrix();
	}
	else{
		glColor3ub(0, 100, 0); //brigth dark green (leaf)
		draw_branch_fifth(size, length, x, y, z);
	}
}
/******************************end of fifth tree function**********************************/
