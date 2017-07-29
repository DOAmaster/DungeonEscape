/* 
 *Name: Cody Graves
 *Last Modified: 7/29/17
 *Project: Dungeon Escape
 * --------------------------------------------------------------------------
 *Week 4: Created SpriteAnimation class and functionality to add generic
 * sprite animations in game. 
 * USAGE: Create SpriteAnimation object somewhere in program, in
 * void init_opengl(void) call animation.convertToPpm(); and 
 * animation.createTexture();, in void movement(Game *game) call
 * animation.enable() and animation.updateAnimation();
 * in void render(Game *game) call renderSprite(animation, x, y);
 * where x and y are the positions you want to render the sprite.
 * --------------------------------------------------------------------------
 *Week 5: Added PlayerState to control player animations. Added control 
 * for facing left to render sprite reversed. 
 * --------------------------------------------------------------------------
 *Week 6: Added Enemy class. Has no functionality with collision, simply moves
 * left and right while cycling through walk animation. Added Unit Test for
 * enemy which verifies it is between left and right stop points.
 * --------------------------------------------------------------------------
 *Week 7: Added enemy vector. Added sprite vector for enemy class. 
 * Added moveEnemy function to move position of enemies as well as movePlayer 
 * function. Added SavePoint class.
 * --------------------------------------------------------------------------
 *Week 8: Added spear class which handles sprite for weapon class. Player may
 * "kill" enemy with this. Spawn player at savepoint when respawning. Changed
 * enemy behavior, including bounds and turning when colliding with object.
 * --------------------------------------------------------------------------
 *Week 9: Added Enemy spawner which clears enemy vector and adds specific
 * enemies depending on the level. Added new enemy type. Added decoration class
 * for background decorations. Added armor upgrade which allows double jump.
 */

#include "codyG.h"

Timers timers;

//---------------------------------------------------------
//SpriteAnimation Class

SpriteAnimation::SpriteAnimation ()
{
	enabled = false;
	spriteSheet = NULL;
	startFrame=currentFrame=maxFrame = -1;
	delay = 0.1;
}

//class constructor
SpriteAnimation::SpriteAnimation(char* name, int maxr, int maxc,
	int numFrames, int sFrame, int eFrame, double framew, double frameh,
	double d, bool l)
{
	enabled = false;
	loop = l;
	sheetName =  name;
	spriteSheet = NULL;
	maxRow = maxr;
	maxColumn = maxc;
	maxFrame = numFrames;
	startFrame = sFrame;
	currentFrame = sFrame;
	endFrame = eFrame;;
	frameWidth = framew;
	frameHeight = frameh;
	delay = d;
	loop = l;

	//calculate current row/column
	int tmpFrame = 0;
	for (int i = 0; i < maxRow; i++) {
		for (int j = 0; j < maxColumn; j++) {
			if (tmpFrame == startFrame) {
				startColumn = currentColumn = j;
				startRow = currentRow = i;
				return;
			}
			tmpFrame ++;
		}
	}
}

//class destructor
SpriteAnimation::~SpriteAnimation() { }

//convert image to usable format
void SpriteAnimation::convertToPpm()
{
	char command[128];

	strcpy(command, "convert ./images/");
	strcat(command, sheetName);
	strcat(command, " ");
	strcpy(sheetNamePpm, sheetName);
	sheetNamePpm[strlen(sheetNamePpm) - 4] = 0;
	strcat(sheetNamePpm, ".ppm");
	strcat(command, sheetNamePpm);

	system(command);
	spriteSheet = ppm6GetImage(sheetNamePpm);
}

//creates texture using ppm image
void SpriteAnimation::createTexture()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	unsigned char *tmpData = buildAlphaData(spriteSheet);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spriteSheet->width,
		spriteSheet->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmpData);
	free(tmpData);
	unlink(sheetNamePpm);
}

//cycles animation through start frame to end frame
void SpriteAnimation::updateAnimation()
{
	if (!enabled)
		return;
	timers.recordTime(&timers.timeCurrent);
	timeSpan = timers.timeDiff(&time, &timers.timeCurrent);
	if (timeSpan > delay) {
		++ currentFrame;
		if (currentFrame > endFrame) {
			if (!loop)
				disable();
			if (startFrame != endFrame)
				currentFrame = startFrame;
			else
				currentFrame = startFrame;
			currentRow = startRow;
			currentColumn = startColumn;
		}
		timers.recordTime(&time);
	}
}

void SpriteAnimation::nextRow() { currentRow += 1; }

void SpriteAnimation::enable() { enabled = true; }

void SpriteAnimation::disable() { enabled = false; }

bool SpriteAnimation::isEnabled() {	return enabled; }

bool SpriteAnimation::isLoop() { return loop; }

int SpriteAnimation::getCurrentFrame() { return currentFrame; }

int SpriteAnimation::getCurrentRow() { return currentRow; }

int SpriteAnimation::getMaxRow() { return maxRow; }

int SpriteAnimation::getMaxColumn() { return maxColumn; }

double SpriteAnimation::getFrameWidth() { return frameWidth; }

double SpriteAnimation::getFrameHeight() { return frameHeight; }

GLuint* SpriteAnimation::getTexture() { return &texture; }

Ppmimage* SpriteAnimation::getSpriteSheet() { return spriteSheet; }

//---------------------------------------------------------
//Enemy Class

//constructor
Enemy::Enemy(int t, Flt w, Flt h, Flt cenX, Flt cenY, Flt hitW, Flt hitH, 
	Flt dirX, Flt dirY, Flt velX, Flt velY, int left, int right, bool l)
{
	type = t;
	s.width = w;
	s.height = h;
	s.center.x = cenX;
	s.center.y = cenY;
	s.center.z = 0.0;
	hitbox.width = hitW;
	hitbox.height = hitH;
	direction.x = dirX;
	direction.y = dirY;
	velocity.x = velX;
	velocity.y = velY;
	leftStop = left;
	rightStop = right;
	isLeft = l;
}

//destructor
Enemy::~Enemy() { }

//checks when enemy should turn around
void Enemy::checkState()
{
	if (velocity.x == 0) {
		velocity.x = 1;
		isLeft = false;
	}
	if (s.center.x >= rightStop || s.center.x <= leftStop) {
		velocity.x = -velocity.x;
		isLeft = !isLeft;
	}
}

bool Enemy::checkIsLeft() { return isLeft; }

Flt Enemy::getX() { return s.center.x; }

Flt Enemy::getY() { return s.center.y; }

void Enemy::setX(Flt x) { s.center.x = x; }

void Enemy::setY(Flt y) { s.center.y = y; }

void Enemy::move() 
{ 
	s.center.x += velocity.x;
	hitbox.center.x = s.center.x;
}

void Enemy::flipDirection() 
{ 
	velocity.x = -velocity.x; 
	if (velocity.x < 0 && type == 0)
		isLeft = true;
	else if (velocity.x > 0 && type == 0)
		isLeft = false;
	if (velocity.x < 0 && type == 1)
		isLeft = false;
	else if (velocity.x > 0 && type == 1)
		isLeft = true;
}

void Enemy::initAnimations()
{
	animations.clear();
	/*
	* TYPES
	* 0 = Zombie
	* 1 = Goblin
	*/
	if (type == 0) {
		SpriteAnimation anim((char*)"zombie.png", 1, 5, 5, 0, 3, 27, 40, 0.1, true);
		animations.push_back(anim);
	}
	if (type == 1) {
		SpriteAnimation anim((char*)"goblin.png", 1, 3, 3, 0, 2, 32, 27, 0.1, true);
		animations.push_back(anim);
	}
}

void Enemy::killEnemy()
{
	this->setX(-100);
	this->setY(this->getY());
	this->velocity.x = 0;
}

void Enemy::spawn(int x, int y)
{
	s.center.x = x;
	s.center.y = y;
	velocity.x = 1;
}

//checks if enemy is between left and right stop values
void Enemy::stateUnitTest()
{
	if (s.center.x > rightStop || s.center.x < leftStop)
		printf("Failure...\n");
	else
		printf("Success!\n");
	printf("Current Position: %f\n", s.center.x);
	printf("Expected Position Between: %i and %i\n", leftStop, rightStop);
}

//---------------------------------------------------------
//SavePoint class
SavePoint::SavePoint(int x, int y, bool e)
{
	xPos = x;
	yPos = y;
	enabled = e;
}

SavePoint::~SavePoint(){ }

void SavePoint::initAnimations()
{
	animations.clear();
	SpriteAnimation anim1((char*)"savepoint.png", 1, 2, 2, 0, 0, 49, 188, 
		0.1, true);
	SpriteAnimation anim2((char*)"savepoint.png", 1, 2, 2, 1, 1, 49, 188,
		0.1, true);
	animations.push_back(anim1);
	animations.push_back(anim2);
}

int SavePoint::getX() { return xPos; } 

int SavePoint::getY() { return yPos; }

bool SavePoint::checkIsEnabled() { return enabled; }

void SavePoint::enable() { enabled = true; }

void SavePoint::disable() { enabled = false; }

//---------------------------------------------------------
// Weapon class

void Spear::initAnimations()
{
	SpriteAnimation anim((char*)"spear.png", 1, 1, 1, 0, 0, 38, 7, 0.1, true);
	sprite = anim;
}

//returns true if left, false if right
void Spear::initSpearDirection(Character p)
{
	if (p.isLeft)
		isLeft = true;
	else
		isLeft = false;
}

bool Spear::checkIsLeft() { return isLeft; }

//---------------------------------------------------------
// Upgrade Class
Upgrade::Upgrade(int x, int y, bool e, string n, string d)
{
	xPos = x;
	yPos = y;
	enabled = e;
	name = n;
	description = d;
}

void Upgrade::initAnimation()
{
	SpriteAnimation anim((char*)"armor.png",1,1,1,0,0,16,27,0.1,true);
	sprite = anim;
}

Upgrade::~Upgrade() { }

int Upgrade::getX() { return xPos; }

int Upgrade::getY() { return yPos; }

bool Upgrade::checkIsEnabled() { return enabled; }

void Upgrade::enable() { enabled = true; }

void Upgrade::disable() { enabled = false; }

//---------------------------------------------------------
// FUNCTIONS
void moveEnemy(Enemy &e, int xpos, int ypos)
{
	e.setX(xpos);
	e.setY(ypos);	
}

void movePlayer(Character &c, int xpos, int ypos)
{
	c.s.center.x = c.hurt.center.x = xpos;
	c.s.center.y = c.hurt.center.x = ypos;
}

void renderSprite(SpriteAnimation sprite, int xposition, 
	int yposition, Flt modifier, bool left)
{
	if (sprite.isEnabled()) {
		int ix;
		float tx, ty, h, w;
		float xPer, yPer;
		SpriteAnimation *tmp = &sprite;
		h = tmp->getFrameHeight() / modifier;
		w = tmp->getFrameWidth() / modifier;
		glPushMatrix();
		glColor3f(1.0, 1.0, 1.0);
		glBindTexture(GL_TEXTURE_2D, *(tmp->getTexture()));
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
		glColor4ub(255,255,255,255);
		ix = tmp->getCurrentFrame() % tmp->getMaxColumn();
		//end of current column, shift down to next row
		if (tmp->getCurrentFrame() > (tmp->getMaxColumn() - 1) * 
			(tmp->getCurrentRow() + 1))
			tmp->nextRow();
		tx = float(ix / float(tmp->getMaxColumn()));
		ty = float(tmp->getCurrentRow() / float(tmp->getMaxRow()));
		xPer = 1.0 / tmp->getMaxColumn();
		yPer = 1.0 / tmp->getMaxRow();
		glBegin(GL_QUADS);
		if (left) {
			glTexCoord2f(tx + xPer,                ty + yPer);
			glVertex2i(w * -1 + xposition,  h * -1 + yposition);
			glTexCoord2f(tx + xPer,                ty);
			glVertex2i(w * -1 + xposition,  h + yposition);
			glTexCoord2f(tx,         ty);
			glVertex2i(w + xposition, h + yposition);
			glTexCoord2f(tx,         ty + yPer);
			glVertex2i(w + xposition, h * -1 + yposition);
		}
		else {
			glTexCoord2f(tx,                ty + yPer);
			glVertex2i(w * -1 + xposition,  h * -1 + yposition);
			glTexCoord2f(tx,                ty);
			glVertex2i(w * -1 + xposition,  h + yposition);
			glTexCoord2f(tx + xPer,         ty);
			glVertex2i(w + xposition, h + yposition);
			glTexCoord2f(tx + xPer,         ty + yPer);
			glVertex2i(w + xposition, h * -1 + yposition);
		}
		glEnd();
		glPopMatrix();
		glDisable(GL_ALPHA_TEST);
	}
}

void updateSpear(Character *p)
{
	for (int i = 0; i < 2; i++) {
		p->l[i].s.center.x += p->l[i].velocity.x;
	}
}


PlayerState getPlayerState(Character *p, char keys[])
{
	PlayerState tmp = STATE_IDLE;
	if (keys[XK_Right] || keys[XK_D] || keys[XK_d] ||
		keys[XK_Left] || keys[XK_A] || keys[XK_a])
		tmp = STATE_RUN;
	if (p->jumpCurrent > 0)
		tmp = STATE_JUMP;
	return tmp;
}


void spawnEntities(int level, vector<Enemy> &e, vector<SavePoint> &s, 
	vector<SpriteAnimation> &d, vector<Upgrade> &u) 
{
	e.clear();
	//disable all savepoint sprites
	for (unsigned int i = 0; i < s.size(); i++) {
		for (int j = 0 ; j < 2; j++) {
			s.at(i).animations.at(j).disable();
		}
	}
	//disable upgrade sprites
	for (unsigned int i = 0; i < u.size(); i++) {
			u.at(i).sprite.disable();
	}

	//spawn decorations on all levels
	SpriteAnimation garg1((char*)"gargoyle.png",1,1,1,0,0,43,38,0.1,true);
	SpriteAnimation garg2((char*)"gargoyle.png",1,1,1,0,0,43,38,0.1,true);
	SpriteAnimation flag1((char*)"flag.png",1,1,1,0,0,30,71,0.1,true);
	SpriteAnimation flag2((char*)"flag.png",1,1,1,0,0,30,71,0.1,true);
	SpriteAnimation torch1((char*)"torch.png",1,1,1,0,0,16,56,0.1,true);
	SpriteAnimation torch2((char*)"torch.png",1,1,1,0,0,16,56,0.1,true);
	SpriteAnimation torch3((char*)"torch.png",1,1,1,0,0,16,56,0.1,true);
	SpriteAnimation shield1((char*)"shield.png",1,1,1,0,0,31,60,0.1,true);
	SpriteAnimation shield2((char*)"shield.png",1,1,1,0,0,31,60,0.1,true);
	d.push_back(garg1);
	d.push_back(garg2);
	d.push_back(flag1);
	d.push_back(flag2);
	d.push_back(torch1);
	d.push_back(torch2);
	d.push_back(torch3);
	d.push_back(shield1);
	d.push_back(shield2);
	

    if (level == 2) {
		Enemy e1(0,27,40,400,48,15,40,0,0,1,0,0,1200,false);
		Enemy e2(0,27,40,300,48,15,40,0,0,-1,0,0,1200,true);
		Enemy e3(0,27,40,500,48,15,40,0,0,-1,0,0,1200,true);
		e.push_back(e1);
		e.push_back(e2);
		e.push_back(e3);
    }

	if (level == 3) {
		Enemy e1(1,32,27,500,48,26,27,0,0,-3,0,0,1200,false);
		e.push_back(e1);
		//Upgrade u1(300, 48, true, "Jumping Armor", "Allows you to double jump");
		//u.push_back(u1);

		//if (s.at(0).checkIsEnabled())
	//		s.at(0).animations.at(1).enable();
//		else
//			s.at(0).animations.at(0).enable();
	}

	if (level == 4) {
		Enemy e1(0,27,40,400,48,15,40,0,0,1,0,0,1200,false);
		Enemy e2(1,32,27,300,48,26,27,0,0,-4,0,0,1200,false);
		e.push_back(e1);
		e.push_back(e2);
	}

	for (unsigned int i = 0; i < e.size(); i++) {
		e.at(i).initAnimations();
		for (unsigned int j = 0; j < e.at(i).animations.size(); j++) {
			e.at(i).animations.at(j).convertToPpm();
			e.at(i).animations.at(j).createTexture();
		}
	}

	for (unsigned int i = 0; i < d.size(); i++) {
		d.at(i).convertToPpm();
		d.at(i).createTexture();
	}

	/*for (unsigned int i = 0; i < u.size(); i++) {
		u.at(i).initAnimation();
		u.at(i).sprite.convertToPpm();
		u.at(i).sprite.createTexture();
	}*/
}

void renderEntities(vector<SpriteAnimation> &d)
{
	if (d.size() < 1)
		return;
	renderSprite(d.at(0), 100, WINDOW_HEIGHT - 100, 1.0, false);
	renderSprite(d.at(1), WINDOW_WIDTH - 100, WINDOW_HEIGHT - 100, 1.0, false);
	renderSprite(d.at(2), 100, WINDOW_HEIGHT - 270, 0.5, false);
	renderSprite(d.at(3), WINDOW_WIDTH - 100, WINDOW_HEIGHT - 270, 0.5, false);
	renderSprite(d.at(4), 100, 200, 2.0, false);
	renderSprite(d.at(5), WINDOW_WIDTH/2, 200, 2.0, false);
	renderSprite(d.at(6), WINDOW_WIDTH - 100, 200, 2.0, false);
	renderSprite(d.at(7), (WINDOW_WIDTH/2 + 100)/2, 270, 1.0, false);
	renderSprite(d.at(8), (WINDOW_WIDTH/2 + (WINDOW_WIDTH-100))/2, 
		270, 1.0, false);
	
}

void checkUpgrade(int level, vector<Upgrade> &u)
{
	for (unsigned int i = 0; i < u.size(); i++) {
			u.at(i).sprite.disable();
	}
	if (level == 3) {
		if (u.at(0).checkIsEnabled())
			u.at(0).sprite.enable();
	}
}

void checkSavePoints(int level, vector<SavePoint> &s)
{
	for (unsigned int i = 0; i < s.size(); i++) {
		for (int j = 0; j < 2; j++) {
			s.at(i).animations.at(j).disable();	
		}	
	}	
	if (level == 3) {
		if (s.at(0).checkIsEnabled())
			s.at(0).animations.at(1).enable();
		else
			s.at(0).animations.at(0).enable();
	}
	if (level == 5) {
		if (s.at(1).checkIsEnabled())
			s.at(1).animations.at(1).enable();
		else
			s.at(1).animations.at(0).enable();
	}
}

int getSavePointLevel(vector<SavePoint> &s) 
{
	if (s.at(0).checkIsEnabled())
		return 3;
	else if (s.at(1).checkIsEnabled())
		return 5;
	return -1;
}

//checks if player collides with upgrade
void upgradeCheck(Character *p, vector<Upgrade> &u)
{
	if (u.size() < 1)
		return;		
	if (u.at(0).checkIsEnabled())
	{
		int x = u.at(0).getX();
		int y = u.at(0).getY();
		int charTop, charBot, charL, charR;
		charTop = y + p->s.height;
		charBot = y - p->s.height;
		charL = x - p->s.width;
		charR = x + p->s.width;
		if (p->s.center.y < charTop && p->s.center.y > charBot) {
			if (p->s.center.x > charL && p->s.center.x < charR) {
				p->upgrade = true;
				p->jumpMax = 2;
				u.at(0).disable();
			}
		}
	}
}
