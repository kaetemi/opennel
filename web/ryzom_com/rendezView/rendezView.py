# -*- coding: utf-8 -*-
import re

from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions
from AccessControl import ClassSecurityInfo
try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *

from config import *

rendezViewSchema=BaseFolderSchema.copy()+ Schema((
	TextField('description',
		widget=TextAreaWidget(
			label='Summary Description',
			label_msgid="rendezView_schema_label_desc",
			description='description of the event',
			description_msgid="rendezView_schema_desc",
			i18n_domain="rendezView",
		),
	),
	TextField('text',
		searchable=True,
		default_output_type='text/html',
		allowable_content_types=(
			'text/restructured',
			'text/plain',
			'text/html',
		),
		widget=RichWidget(
			label="",
			label_msgid="rendezView_schema_label_text",
			description="Description de l'évenement",
			description_msgid="rendezView_schema_text",
			i18n_domain="rendezView",
		)
	),
	DateTimeField('date',
		required=True,
		widget=CalendarWidget(
			label="",
			label_msgid="rendezView_schema_label_date",
			description="date de l'evenement",			
			description_msgid="rendezView_schema_date",
			i18n_domain="rendezView",			
		)
	),
	DateTimeField('dateLimit',
		required=True,
		widget=CalendarWidget(
			label="",
			label_msgid="rendezView_schema_label_datelimit",
			description="date limite d'inscription",
			description_msgid="rendezView_schema_datelimit",
			i18n_domain="rendezView",
		)
	),
	IntegerField('nbSeat',
		default=1,
		widget=IntegerWidget(
			label="",
			label_msgid="rendezView_schema_label_nbSeat",
			description="Nombre de place disponible",
			description_msgid="rendezView_schema_nbseat",
			i18n_domain="rendezView",
		)
	),
	IntegerField('maxSeat',
		default=1,
		widget=IntegerWidget(
			label="",
			label_msgid="rendezView_schema_label_maxseat",
			description="Nombre de place réservable au maximum par personne",
			description_msgid="rendezView_schema_maxseat",
			i18n_domain="rendezView",
		)
	),
	BooleanField('visibleSeat',
		default=1,
		widget=BooleanWidget(
			label="",
			label_msgid="rendezView_schema_label_visibleseat",
			description="Nombres de places visibles ?",
			description_msgid="rendezView_schema_visibleseat",
			i18n_domain="rendezView",
		)
	),
	TextField('address',
		default_output_type='text/html',
		allowable_content_types=(
			'text/restructured',
			'text/plain',
			'text/html',
		),
		widget=RichWidget(
			label="",
			label_msgid="rendezView_schema_label_address",
			description="lieu de l'evenement",
			description_msgid="rendezView_schema_address",
			i18n_domain="rendezView",
		),
		schemata='address',
	),
	ImageField('plan',
		widget=ImageWidget(
			label="",
			label_msgid="rendezView_schema_label_plan",
			description="Inserer un plan",			
			description_msgid="rendezView_schema_plan",
			i18n_domain="rendezView",
		),
		schemata='address',
	),
#	LinesField('participant',		
#		widget=LinesWidget(),
#		visible={'edit':'hidden', 'view':'visible'},
#	),
),)
 
 
   
class rendezView(BaseFolder):

	"""Add an rendezView event"""
	schema = rendezViewSchema
	security = ClassSecurityInfo()
	archetype_name = "rendezView"
	meta_type = 'rendezView'
	default_view  = 'rendezView_view'
	immediate_view = 'rendezView_view'
	allowed_content_types = ('participant',)
	_at_rename_after_creation = True

	actions = (
		{
		'id': 'view',
		'name': 'view',
		'action': 'string:${object_url}/rendezView_view',
		'permissions': (CMFCorePermissions.View,)
		},
		{
		'id': 'listing',
		'name': 'Listing',
		'action': 'string:${folder_url}/folder_contents',
		'permissions': (CMFCorePermissions.ModifyPortalContent,)
		},
#		{
#		'id': 'register',
#		'name': 'register',
#		'action': 'string:${object_url}/addParticipant',
#		'permission': (CMFCorePermissions.View,)
#		},
	)

# 	replace fonction by """_at_rename_after_creation = True"""
#	def setTitle(self, value, **kwargs):
#		if not value and self.id:
#			value = self.id
#		else:			
#			try:
#				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
#			except:
#				pass
#		self.getField('title').set(self, value, **kwargs)


	def isAvailable(self):
		"""test si l'utilisateur peut s'inscrire"""
		return (self.nbSeatsTake() < self.getNbSeat()) and (not self.isRegister())

	def isRegister(self):
		"""test si l'utilisateur est déja inscrit"""
		#on recupere le membre
		mtool = getToolByName(self, 'portal_membership')
		inscriptId = self.getId()+'_'+mtool.getAuthenticatedMember().getUserName()
		path = '/'.join(self.getPhysicalPath())
		if self.portal_catalog(id=inscriptId,meta_type=['participant',],path={'query':path, 'level': 0},):
			return True
		return False

	security.declarePublic('nbSeatsTake')
	def nbSeatsTake(self):
		"""renvoie les place prises"""
		s = 0
		tab = self.getParticipant()
		for i in tab:
			o = i.getObject()
			s += o.getSeat()
		return s


	def nbParticipant(self):
		"""renvoie le nombre d'inscriptions"""		
		tab = self.getParticipant()
		return len(tab)
			
	security.declarePublic('getParticipant')
	def getParticipant(self):
		"""renvoie la listes des participants"""
		path = '/'.join(self.getPhysicalPath())
		results = self.portal_catalog(meta_type=['participant',],path={'query':path, 'level': 0},)
		return results


	security.declarePublic('addParticipant')
	def addParticipant(self,seat=1,comment='',pseudo=''):
		"""ajoute une inscription"""
		if not (self.isAvailable()):
			return "[Plus d'inscription disponible]"
		path = '/'.join(self.getPhysicalPath())
		mtool = getToolByName(self, 'portal_membership')
		member = mtool.getAuthenticatedMember()
		fullname = member.getProperty('fullname')
		login = member.getUserName()
		email = member.getProperty('email')
		seat = seat
		comment = comment
		pseudo = pseudo
		newid = self.getId()+'_'+login
		title = login
		try:
			self.invokeFactory(type_name='participant', id=newid, title=title, description='', fullName=fullname, email=email, seat=seat)
			new_obj = getattr(self, newid)
			new_obj.setFullName(fullname)
			new_obj.setLogin(login)
			new_obj.setPseudo(pseudo)
			new_obj.setEmail(email)
			new_obj.setSeat(seat)
			new_obj.setComment(comment)
			return "[inscription success]"
		except:
			return "[inscription deja faite]"

	security.declarePublic('removeParticipant')
	def removeParticipant(self,REQUEST):
		"""Permet à un inscrit de se désinscrire"""
		#on récupère l'objet participant correspondant
		mtool = getToolByName(self, 'portal_membership')
		inscriptId = self.getId()+'_'+mtool.getAuthenticatedMember().getUserName()			

		#on supprime l'objet
		self.manage_delObjects(inscriptId)
		REQUEST['RESPONSE'].redirect(self.absolute_url())



	def nbSeatsRestant(self):
		"""retourne le nombre de places restantes"""
		return self.getNbSeat()-self.nbSeatsTake()

	security.declarePublic('seats')
	def seats(self):
		"""retourne place prise / place totale"""
		return str(self.nbSeatsTake())+'/'+str(self.getNbSeat())


registerType(rendezView,PROJECTNAME)
