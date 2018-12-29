#!/usr/bin/env python
"""
@file    test.py
@author  Pablo Alvarez Lopez
@date    2016-11-25
@version $Id: test.py 24076 2017-04-26 17:49:30Z palcraft $

python script used by sikulix for testing netedit

SUMO, Simulation of Urban MObility; see http://sumo.dlr.de/
Copyright (C) 2009-2017 DLR/TS, Germany

This file is part of SUMO.
SUMO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
"""
# import common functions for netedit tests
import os
import sys

testRoot = os.path.join(os.environ.get('SUMO_HOME', '.'), 'tests')
neteditTestRoot = os.path.join(
    os.environ.get('TEXTTEST_HOME', testRoot), 'netedit')
sys.path.append(neteditTestRoot)
import neteditTestFunctions as netedit

# Open netedit
neteditProcess, match = netedit.setupAndStart(neteditTestRoot, False)

# go to additional mode
netedit.additionalMode()

# select routeProbe
netedit.changeAdditional("routeProbe")

# create routeProbe
netedit.leftClick(match, 250, 120)

# create another routeProbe (for duplicated ID)
netedit.leftClick(match, 250, 220)

# go to inspect mode
netedit.inspectMode()

# inspect first routeProbe
netedit.leftClick(match, 310, 200)

# Change parameter 0 with a non valid value (Duplicated ID)
netedit.modifyAttribute(0, "routeProbe_3_1")

# Change parameter 0 with a valid value
netedit.modifyAttribute(0, "correct ID")

# Change parameter 1 with a non valid value (dummy edge)
netedit.modifyAttribute(1, "dummyEdge")

# Change parameter 1 with a valid value (different edge)
netedit.modifyAttribute(1, "gneE0")

# Change parameter 2 with a non valid value (dummy)
netedit.modifyAttribute(2, "dummy")

# Change parameter 2 with a non valid value (negative)
netedit.modifyAttribute(2, "-50")

# Change parameter 2 with a valid value
netedit.modifyAttribute(2, "150")

# Change parameter 3 with a non valid value (dummy)
netedit.modifyAttribute(4, "")

# Change parameter 3 with a valid value
netedit.modifyAttribute(4, "OwnOutput.txt")

# Change parameter 4 with a non valid value (dummy)
netedit.modifyAttribute(4, "dummy")

# Change parameter 4 with a non valid value (negative)
netedit.modifyAttribute(4, "-5")

# Change parameter 4 with a valid value
netedit.modifyAttribute(4, "20")

# click over an empty area
netedit.leftClick(match, 0, 0)

# Check undos and redos
netedit.undo(match, 13)
netedit.redo(match, 13)

# save additionals
netedit.saveAdditionals()

# save newtork
netedit.saveNetwork()

# quit netedit
netedit.quit(neteditProcess)