#
# Authors: Brian Skahan <bskahan@etria.com>
#          Tom von Schwerdtner <tvon@etria.com>
#
# Copyright 2004-2005, Etria, LLP
#
# This file is part of Quills
#
# Quills is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Quills is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Quills; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



from Products.CMFCore.CMFCorePermissions import AddPortalContent
from Products.Archetypes.public import DisplayList

ADD_CONTENT_PERMISSION = AddPortalContent
PROJECTNAME = "Quills"
SKINS_DIR = "skins"

GLOBALS = globals()

STYLES = {'light':'Light',
          'noimage':'No Images',
          'standard':'Standard'
         }
