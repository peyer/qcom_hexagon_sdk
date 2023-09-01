/***************************************************************************
* Copyright (c) Date: Tue Aug 26 16:58:15 CDT 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Tue Aug 26 16:58:15 CDT 2008
****************************************************************************/
#include <iostream>
#include <ctime>
#include <math.h>

#include <hexagon_sim_timer.h>

using namespace std;
#define N 100
#define G 6.673e-11
#define TIMESTAMP 1e11

struct Particle
{
    double rx, ry;//position components
    double vx, vy;//velocity components
    double fx, fy;//force components
    double mass;//mass of the particle
};

Particle Update(Particle p, double timestamp)
{
    p.vx += timestamp*p.fx / p.mass;
    p.vy += timestamp*p.fy / p.mass;
    p.rx += timestamp*p.vx;
    p.ry += timestamp*p.vy;
    return p;
}

//Reset the forces on particle
Particle ResetForce(Particle p)
{
    p.fx = 0.0;
    p.fy = 0.0;
    return p;
}

//Add force to particle a by particle b
Particle AddForce(Particle a,Particle b)
{
    double EPS = 3E4;      // softening parameter (just to avoid infinities)
    double dx = b.rx - a.rx;
    double dy = b.ry - a.ry;
    double dist = sqrt(dx*dx + dy*dy);
    double F = (G * a.mass * b.mass) / (dist*dist + EPS*EPS);
    a.fx += F * dx / dist;
    a.fy += F * dy / dist;
    return a;
}

int main(int argc, char **argv)
{
	unsigned long long start;
    int numberofiterations = 10;
    int count = 0;

    Particle particles[N];
    srand(time(NULL));

	start = hexagon_sim_read_pcycles();

	//randomly generating N Particles
    for (int i = 0; i < N; i++)
	{
        double rx = 1e18*exp(-1.8)*(.5 - rand());
        particles[i].rx = rx;
        double ry = 1e18*exp(-1.8)*(.5 - rand());
        particles[i].ry = ry;
        double vx = 1e18*exp(-1.8)*(.5 - rand());
        particles[i].vx = vx;
        double vy = 1e18*exp(-1.8)*(.5 - rand());
        particles[i].vy = vy;
        double mass = 1.98892e30*rand()*10 + 1e20;
        particles[i].mass = mass;
    }


	while (count < numberofiterations)
	{
        for (int i = 0; i < N; i++)
        {
            particles[i] = ResetForce(particles[i]);
            for (int j = 0; j < N; j++)
            {
                if (i != j)
                {
                    particles[i] = AddForce(particles[i], particles[j]);
                }

            }
        }

        //loop again to update the time stamp here
        for (int i = 0; i < N; i++)
        {
            particles[i] = Update(particles[i], TIMESTAMP);
        }
        count++;
    }

	printf("\n\nTotal pcycles = %llu\n\n", hexagon_sim_read_pcycles() - start);

    return 0;
}
