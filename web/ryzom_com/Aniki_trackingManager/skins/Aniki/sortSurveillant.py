## Script (Python) "sortSurveillant"
##bind container=container
##bind context=context
##bind namespace=
##bind script=script
##bind subpath=traverse_subpath
##parameters=filter=None, capoZ, format=None,text=None
##title=
##
def filter_format(format,capo):
	format = format.split(',')
	if 'all' in format:
		return capo
	else:
		newcapoZ = []
		for brain in capo:
			obj = brain.getObject()
			if format[0].upper() in str(obj.getDescription().upper()):
				newcapoZ.append(brain)
		return newcapoZ

if filter:
	filter = filter.split(',')
	if 'all' in filter:
		newcapoZ = capoZ
	else:
		url = []
		lang_url = {'en':'newcomer/game-download','fr':'nouveaux-venus/telecharger-le-jeu','de':'neuling-s-inn/spiel-download'}
		for i in filter:
			if i in lang_url.keys():
				url.append(lang_url[i])
		newcapoZ = []
		for brain in capoZ:
			obj = brain.getObject()
			for u in url:
				if u in str(obj.getUrlRedirection()):
					newcapoZ.append(brain)
	if format:
		newcapoZ = filter_format(format,newcapoZ)
	if text:
		newcapoZ = filter_format(text,newcapoZ)
	return newcapoZ

if format:
	newcapoZ = filter_format(format,capoZ)
	if text:
		newcapoZ = filter_format(text,newcapoZ)
	return newcapoZ
if text:
	return filter_format(text,capoZ)

return capoZ
