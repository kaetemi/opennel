var GLOBAL_ID_TO_MOD = '';

function MyHttpObject(id){
	this.callback = function changeContentURL(idDom){
	if (xmlhttp.readyState == 4){
		if (xmlhttp.status == 200){
			changeContentOf(GLOBAL_ID_TO_MOD, xmlhttp.responseText);
		}
	}
}

function changeContentOf(idDom, newContent){
	element = document.getElementById(idDom);
	element.innerHTML = newContent;
}
	this.id = id;
	

}



function getHTTPObject(func){
	var xmlhttp = false;
	
	/* Compilation conditionnelle d'IE */
	/*@cc_on
	@if (@_jscript_version >= 5)
		try{
			xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e){
			try{
				xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch (E){
				xmlhttp = false;
			}
		}
	@else
	
	xmlhttp = false;
	
	@end @*/
	
	/* on essaie de cr�er l'objet si ce n'est pas d�j� fait */
	if (!xmlhttp && typeof XMLHttpRequest != 'undefined'){
		try{
			xmlhttp = new XMLHttpRequest();
		}
		catch (e){
			xmlhttp = false;
		}
	}
	
	if (xmlhttp){
		/* on d�finit ce qui doit se passer quand la page r�pondra */
		xmlhttp.onreadystatechange = func();
	}
	return xmlhttp;
}

function example(){
	/*Verifie si l'etat est bien 4 (fini)*/
	if (xmlhttp.readyState == 4){
		/* 200 : code HTTP pour OK */
		if (xmlhttp.status == 200){
			/*
			Traitement de la r�ponse.
			Ici on affiche la r�ponse dans une bo�te de dialogue.
			*/
			alert(xmlhttp.responseText);
		}
	}
}



function sendRequestAndChangeContent(idDom, url){
	var httpTruc = getHTTPObject();
	
}