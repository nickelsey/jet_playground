Welcome! This is a small project to learn some more about machine learning techniques
using monte-carlo generated proton-proton collisions simulated in STAR, a detector
at RHIC. We have both PYTHIA ( a monte-carlo event generator ) and GEANT ( a
detector response simulation ) data. The Geant data corresponds to running the 
PYTHIA events through GEANT, then through the STAR event reconstruction chain.

My first goal to to try to be able to predict, given a GEANT event, kinematics
of the initial PYTHIA event before the simulation. I study jets ( highly collimated
sprays of energetic particles caused by parton hard scatterings ), so my first 
goal is to be able to "correct" a GEANT jet to the level of the PYTHIA jet, given
any event kinematics that would be available to someone working on recorded data.

I'll update this as it goes along

I'm currently working on: faster pipelining for models and getting tensorflow up 
and running
