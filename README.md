# apollo-rtos

Most modern real-time operating systems for small micro-controllers, such as those using an ARM M0+ processor, allow for a small number of processes which are defined at compile time. In contrast, one of the first examples of a real-time operating system, deployed on the Apollo Guidance Computer (AGC), was more sophisticated. It allowed short WAITLIST tasks to be scheduled at set intervals. Longer EXECUTIVE jobs could be created with different priorities at runtime, typically by the WAITLIST tasks themselves, and these would be completed using whatever spare processor time was available. 

Famously, during the Apollo 11 landing, the AGC running in the lunar lander ran out of space for new EXECUTIVE jobs, triggering a 1202 alarm. Although low priority jobs updating the astronauts’ displays failed to run, the higher priority jobs controlling the space craft continued as normal and the crew landed safely. This approach to real-time operating system design appears to still offer some advantages. To explore these advantages, in this project, you will develop a real-time operating system for an ARM M0+ processor modelled on this approach.

