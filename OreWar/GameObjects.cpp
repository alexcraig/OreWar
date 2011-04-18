#include "GameObjects.h"

// ========================================================================
// BaseObject Implementation
// ========================================================================

BaseObject::BaseObject(float x, float y) : m_xPos(x), m_yPos(y) {
}

BaseObject::BaseObject() : m_xPos(0), m_yPos(0) {
}

void BaseObject::setXPos(float newX) {
	m_xPos = newX;
}

void BaseObject::setYPos(float newY) {
	m_yPos = newY;
}

float BaseObject::getXPos() {
	return m_xPos;
}

float BaseObject::getYPos() {
	return m_yPos;
}

BaseObject::~BaseObject() {
}

// ========================================================================
// BaseObject Implementation
// ========================================================================

PhysicsObject::PhysicsObject(float mass) : BaseObject(0, 0), m_xVel(0), m_yVel(0),
	m_xAccel(0), m_yAccel(0), m_xForce(0), m_yForce(0), m_mass(mass) {
}

PhysicsObject::PhysicsObject(float mass, float xPos, float yPos)  : BaseObject(xPos, yPos), m_xVel(0),
	m_yVel(0), m_xAccel(0), m_yAccel(0), m_xForce(0), m_yForce(0), m_mass(mass) {
}

void PhysicsObject::setXVel(float newX) {
	m_xVel = newX;
}

void PhysicsObject::setYVel(float newY) {
	m_yVel = newY;
}

float PhysicsObject::getXVel() {
	return m_xVel;
}

float PhysicsObject::getYVel() {
	return m_yVel;
}

void PhysicsObject::setXAccel(float newX) {
	m_xAccel = newX;
}

void PhysicsObject::setYAccel(float newY) {
	m_yAccel = newY;
}

float PhysicsObject::getXAccel() {
	return m_xAccel;
}

float PhysicsObject::getYAccel() {
	return m_yAccel;
}

void PhysicsObject::applyForce(float xForce, float yForce) {
	m_xForce += xForce;
	m_yForce += yForce;
}

void PhysicsObject::clearForces() {
	m_xForce = 0;
	m_yForce = 0;
}

void PhysicsObject::updatePosition(float timeElapsed) {
	// if(m_mass == 0) {
	// TODO: Throw exception
	// }

	m_xAccel = (m_xForce / m_mass);
	m_yAccel = (m_yForce / m_mass);
	m_xVel += m_xAccel;
	m_yVel += m_yAccel;
	setXPos(getXPos() + (m_xVel * timeElapsed));
	setYPos(getYPos() + (m_yVel * timeElapsed));
}