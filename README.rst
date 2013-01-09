=====
OVNIS
=====

:Date: Jan 16, 2011
:Author: Yoann Pigné
:Organization: University of Luxembourg
:Contact: yoann@pigne.org
:Version: 0.1
:Copyright: 2010-2011, University of Luxembourg

.. This document is a general introduction to the project. Check the wiki for more information. 

OVNIS stands for Online Vehicular Network Integrated Simulation. It is a platform dedicated to the simulation of vehicular network applications. OVNIS integrates network simulator `ns-3`_ and traffic microsimulator `SUMO`_. Both simulators are coupled so that the mobility of vehicles in SUMO is injected in the mobility model of ns-3. Inversely, any simulated network application in ns-3 can influence the traffic simulation and, for instance, reroute simulated vehicles. 


For more information please check the `Wiki`_ pages. You may find a `definition of the architecture`_. You may also get inputs on how to build the platform. 

Install
-------

Follow this `installation guide`_ from the Wiki section. 


Researchers
-----------

if you use this project, please cite: 

Yoann Pigné, Grégoire Danoy and Pascal Bouvry. ``A Platform for Realistic Online Vehicular Network Management``. In `IEEE International Workshop on Management of Emerging Networks and Services`, Pages 615-619. IEEE Computer Society, 2010.

::

 @inproceedings{Pigne:2010kl,
   Author = {Pigné, Yoann and Danoy, Grégoire and Bouvry, Pascal},
   Booktitle = {IEEE International Workshop on Management of Emerging Networks and Services},
   Pages = {615-619},
   Isbn={978-1-4244-8864-3},  
   Publisher = {IEEE Computer Society},
   Title = {A Platform for Realistic Online Vehicular Network Management},
   Year = {2010},
 }


.. _Wiki: https://github.com/pigne/ovnis/wiki
.. _ns-3: http://www.nsnam.org/
.. _SUMO: http://sumo.sourceforge.net/
.. _installation guide: https://github.com/pigne/ovnis/wiki/Install
.. _definition of the architecture: https://github.com/pigne/ovnis/wiki/ArchitectureDefinition