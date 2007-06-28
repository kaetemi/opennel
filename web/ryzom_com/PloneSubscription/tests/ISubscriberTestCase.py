# -*- coding: utf-8 -*-
## PloneSubscription
## A Plone tool supporting different levels of subscription and notification
## Copyright (C)2006 Ingeniweb

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; see the file COPYING. If not, write to the
## Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
"""
This module implements subscriber interface testcase

We use normal inheritance and not normal interface here
"""
__version__ = "$Revision: 1.2 $"
# $Source: /home/cvsroot/code/web/ryzom_com/PloneSubscription/tests/ISubscriberTestCase.py,v $
# $Id: ISubscriberTestCase.py,v 1.2 2006-04-19 14:36:55 bernard Exp $
__docformat__ = 'restructuredtext'

class ISubscriberTestCase:
    """Subscriber Interface"""

    def test_getUsers(self):
        """Return users concerned by the subscriber"""
        raise NotImplementedError
