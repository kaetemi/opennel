/*********************************************************************NVMH2****
File:  NvTriStripObjects.cpp

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:


******************************************************************************/


#pragma warning( disable : 4786 )  

#include "nel/misc/debug.h"
#include <set>
#include <algorithm>
#include <cstdio>
#include "nv_tri_strip_objects.h"
#include "nv_vertex_cache.h"


using namespace std;

NvStripifier::NvStripifier() 
{

}

NvStripifier::~NvStripifier()
{

}

///////////////////////////////////////////////////////////////////////////////////////////
// FindEdgeInfo()
//
// find the edge info for these two indices
//
NvEdgeInfo * NvStripifier::FindEdgeInfo(NvEdgeInfoVec &edgeInfos, int v0, int v1){

  // we can get to it through either array
  // because the edge infos have a v0 and v1
  // and there is no order except how it was
  // first created.
  NvEdgeInfo *infoIter = edgeInfos[v0];
  while (infoIter != NULL){
    if (infoIter->m_v0 == v0){
      if (infoIter->m_v1 == v1)
      	return infoIter;
      else
      	infoIter = infoIter->m_nextV0;
    }
    else {
      nlassert(infoIter->m_v1 == v0);
      if (infoIter->m_v0 == v1)
      	return infoIter;
      else
      	infoIter = infoIter->m_nextV1;
    }
  }
  return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////
// FindOtherFace
//
// find the other face sharing these vertices
// exactly like the edge info above
//
NvFaceInfo * NvStripifier::FindOtherFace(NvEdgeInfoVec &edgeInfos, int v0, int v1, NvFaceInfo *faceInfo){
  NvEdgeInfo *edgeInfo = FindEdgeInfo(edgeInfos, v0, v1);
  nlassert(edgeInfo != NULL);
  return (edgeInfo->m_face0 == faceInfo ? edgeInfo->m_face1 : edgeInfo->m_face0);
}


///////////////////////////////////////////////////////////////////////////////////////////
// BuildStripifyInfo()
//
// Builds the list of all face and edge infos
//
void NvStripifier::BuildStripifyInfo(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos){
  
  // reserve space for the face infos, but do not resize them.
  int numIndices = indices.size();
  faceInfos.reserve(numIndices);
  int	i;

  // we actually resize the edge infos, so we must initialize to NULL
  int	maxIndex=0;
  for (i = 0; i < numIndices; i++)
	  maxIndex= max(maxIndex, (int)indices[i]);
  edgeInfos.resize (maxIndex+1);
  for (i = 0; i < maxIndex+1; i++)
    edgeInfos[i] = NULL;
  
  // iterate through the triangles of the triangle list
  int numTriangles = numIndices / 3;
  int index        = 0;
  for (i = 0; i < numTriangles; i++){
    
    // grab the indices
    int v0 = indices[index++];
    int v1 = indices[index++];
    int v2 = indices[index++];
    
    // create the face info and add it to the list of faces
    NvFaceInfo *faceInfo = new NvFaceInfo(v0, v1, v2);
    faceInfos.push_back(faceInfo);
    
    // grab the edge infos, creating them if they do not already exist
    NvEdgeInfo *edgeInfo01 = FindEdgeInfo(edgeInfos, v0, v1);
    if (edgeInfo01 == NULL){
      
      // create the info
      edgeInfo01 = new NvEdgeInfo(v0, v1);
      
      // update the linked list on both 
      edgeInfo01->m_nextV0 = edgeInfos[v0];
      edgeInfo01->m_nextV1 = edgeInfos[v1];
      edgeInfos[v0] = edgeInfo01;
      edgeInfos[v1] = edgeInfo01;
      
      // set face 0
      edgeInfo01->m_face0 = faceInfo;
    }
    else {
        if (edgeInfo01->m_face1 != NULL){
          printf("BuildStripifyInfo: > 2 triangles on an edge... uncertain consequences\n");
        }
      edgeInfo01->m_face1 = faceInfo;
    }
    
    // grab the edge infos, creating them if they do not already exist
    NvEdgeInfo *edgeInfo12 = FindEdgeInfo(edgeInfos, v1, v2);
    if (edgeInfo12 == NULL){
      
      // create the info
      edgeInfo12 = new NvEdgeInfo(v1, v2);
      
      // update the linked list on both 
      edgeInfo12->m_nextV0 = edgeInfos[v1];
      edgeInfo12->m_nextV1 = edgeInfos[v2];
      edgeInfos[v1] = edgeInfo12;
      edgeInfos[v2] = edgeInfo12;
      
      // set face 0
      edgeInfo12->m_face0 = faceInfo;
    }
    else {
        if (edgeInfo12->m_face1 != NULL){	        
          printf("BuildStripifyInfo: > 2 triangles on an edge... uncertain consequences\n");
        }
      edgeInfo12->m_face1 = faceInfo;
    }
    
    // grab the edge infos, creating them if they do not already exist
    NvEdgeInfo *edgeInfo20 = FindEdgeInfo(edgeInfos, v2, v0);
    if (edgeInfo20 == NULL){
      
      // create the info
      edgeInfo20 = new NvEdgeInfo(v2, v0);
      
      // update the linked list on both 
      edgeInfo20->m_nextV0 = edgeInfos[v2];
      edgeInfo20->m_nextV1 = edgeInfos[v0];
      edgeInfos[v2] = edgeInfo20;
      edgeInfos[v0] = edgeInfo20;
      
      // set face 0
      edgeInfo20->m_face0 = faceInfo;
    }
    else {
        if (edgeInfo20->m_face1 != NULL){
          printf("BuildStripifyInfo: > 2 triangles on an edge... uncertain consequences\n");
        }
      edgeInfo20->m_face1 = faceInfo;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////////////////
// FindStartPoint()
//
// Finds a good starting point, namely one which has only one neighbor
//
int NvStripifier::FindStartPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos)
{
  for(int i = 0; i < faceInfos.size(); i++)
  {
    int ctr = 0;

    if(FindOtherFace(edgeInfos, faceInfos[i]->m_v0, faceInfos[i]->m_v1, faceInfos[i]) == NULL)
      ctr++;
    if(FindOtherFace(edgeInfos, faceInfos[i]->m_v1, faceInfos[i]->m_v2, faceInfos[i]) == NULL)
      ctr++;
    if(FindOtherFace(edgeInfos, faceInfos[i]->m_v2, faceInfos[i]->m_v0, faceInfos[i]) == NULL)
      ctr++;
    if(ctr > 1)
      return i;
  }
  return -1;
}

  
///////////////////////////////////////////////////////////////////////////////////////////
// FindGoodResetPoint()
//  
// A good reset point is one near other commited areas so that
// we know that when we've made the longest strips its because
// we're stripifying in the same general orientation.
//
NvFaceInfo* NvStripifier::FindGoodResetPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos){

  // we hop into different areas of the mesh to try to get
  // other large open spans done.  Areas of small strips can
  // just be left to triangle lists added at the end.
  static float meshJump = 0.0f;
  static bool firstTime = true;

  NvFaceInfo *result = NULL;
  
  if(result == NULL)
  {
    int numFaces   = faceInfos.size();
    int startPoint;
    if(firstTime)
    {
      //first time, find a face with few neighbors (look for an edge of the mesh)
      startPoint = FindStartPoint(faceInfos, edgeInfos);
      firstTime = false;
    }
    else
      startPoint = (int)(((float) numFaces - 1) * meshJump);

    if(startPoint == -1)
      startPoint = (int)(((float) numFaces - 1) * meshJump);

    int i = startPoint;
    do {
  
      // if this guy isn't visited, try him
      if (faceInfos[i]->m_stripId < 0){
        result = faceInfos[i];
        break;
      }
  
      // update the index and clamp to 0-(numFaces-1)
      if (++i >= numFaces)
        i = 0;
  
    } while (i != startPoint);
  
    // update the 'meshJump
    meshJump += 0.1f;
    if (meshJump > 1.0f)
      meshJump = .05f;
  }
  
  // return the best face we found
  return result;
}


///////////////////////////////////////////////////////////////////////////////////////////
// GetUniqueVertexInB()
//
// Returns the vertex unique to faceB
//
int NvStripifier::GetUniqueVertexInB(NvFaceInfo *faceA, NvFaceInfo *faceB){
  
  int facev0 = faceB->m_v0;
  if (facev0 != faceA->m_v0 &&
      facev0 != faceA->m_v1 &&
      facev0 != faceA->m_v2)
    return facev0;
  
  int facev1 = faceB->m_v1;
  if (facev1 != faceA->m_v0 &&
      facev1 != faceA->m_v1 &&
      facev1 != faceA->m_v2)
    return facev1;
  
  int facev2 = faceB->m_v2;
  if (facev2 != faceA->m_v0 &&
      facev2 != faceA->m_v1 &&
      facev2 != faceA->m_v2)
    return facev2;
  
  // nothing is different
  return -1;
}


///////////////////////////////////////////////////////////////////////////////////////////
// GetSharedVertex()
//
// Returns the vertex shared between the two input faces
//
int NvStripifier::GetSharedVertex(NvFaceInfo *faceA, NvFaceInfo *faceB){
  
  int facev0 = faceB->m_v0;
  if (facev0 == faceA->m_v0 ||
      facev0 == faceA->m_v1 ||
      facev0 == faceA->m_v2)
    return facev0;
  
  int facev1 = faceB->m_v1;
  if (facev1 == faceA->m_v0 ||
      facev1 == faceA->m_v1 ||
      facev1 == faceA->m_v2)
    return facev1;
  
  int facev2 = faceB->m_v2;
  if (facev2 == faceA->m_v0 ||
      facev2 == faceA->m_v1 ||
      facev2 == faceA->m_v2)
    return facev2;
  
  // nothing is shared
  return -1;
}


///////////////////////////////////////////////////////////////////////////////////////////
// GetNextIndex()
//
// Returns vertex of the input face which is "next" in the input index list
//
inline int NvStripifier::GetNextIndex(const WordVec &indices, NvFaceInfo *face){

  int numIndices = indices.size();
  nlassert(numIndices >= 2);

  int v0  = indices[numIndices-2];
  int v1  = indices[numIndices-1];

  int fv0 = face->m_v0;
  int fv1 = face->m_v1;
  int fv2 = face->m_v2;

  if (fv0 != v0 && fv0 != v1){
    if ((fv1 != v0 && fv1 != v1) || (fv2 != v0 && fv2 != v1)){
      printf("GetNextIndex: Triangle doesn't have all of its vertices\n");
      printf("GetNextIndex: Duplicate triangle probably got us derailed\n");
    }
    return fv0;
  }
  if (fv1 != v0 && fv1 != v1){
    if ((fv0 != v0 && fv0 != v1) || (fv2 != v0 && fv2 != v1)){
      printf("GetNextIndex: Triangle doesn't have all of its vertices\n");
      printf("GetNextIndex: Duplicate triangle probably got us derailed\n");
    }
    return fv1;
  }
  if (fv2 != v0 && fv2 != v1){
    if ((fv0 != v0 && fv0 != v1) || (fv1 != v0 && fv1 != v1)){
      printf("GetNextIndex: Triangle doesn't have all of its vertices\n");
      printf("GetNextIndex: Duplicate triangle probably got us derailed\n");
    }
    return fv2;
  }

  // shouldn't get here
  printf("GetNextIndex: Duplicate triangle sent\n");
  return -1;
}


///////////////////////////////////////////////////////////////////////////////////////////
// IsMarked()
//
// If either the faceInfo has a real strip index because it is
// already assign to a committed strip OR it is assigned in an
// experiment and the experiment index is the one we are building
// for, then it is marked and unavailable
inline bool NvStripInfo::IsMarked(NvFaceInfo *faceInfo){
  return (faceInfo->m_stripId >= 0) || (IsExperiment() && faceInfo->m_experimentId == m_experimentId);
}


///////////////////////////////////////////////////////////////////////////////////////////
// MarkTriangle()
//
// Marks the face with the current strip ID
//
inline void NvStripInfo::MarkTriangle(NvFaceInfo *faceInfo){
  nlassert(!IsMarked(faceInfo));
  if (IsExperiment()){
    faceInfo->m_experimentId = m_experimentId;
    faceInfo->m_testStripId  = m_stripId;
    }
  else{
    nlassert(faceInfo->m_stripId == -1);
    faceInfo->m_experimentId = -1;
    faceInfo->m_stripId      = m_stripId;
  }
}


///////////////////////////////////////////////////////////////////////////////////////////
// Build()
//
// Builds a strip forward as far as we can go, then builds backwards, and joins the two lists
//
void NvStripInfo::Build(NvEdgeInfoVec &edgeInfos, NvFaceInfoVec &faceInfos){

  // used in building the strips forward and backward
  static WordVec scratchIndices;
  scratchIndices.resize(0);
  
  // build forward... start with the initial face
  NvFaceInfoVec forwardFaces, backwardFaces;
  forwardFaces.push_back(m_startInfo.m_startFace);
  MarkTriangle(m_startInfo.m_startFace);
  
  int v0 = (m_startInfo.m_toV1 ? m_startInfo.m_startEdge->m_v0 : m_startInfo.m_startEdge->m_v1);
  int v1 = (m_startInfo.m_toV1 ? m_startInfo.m_startEdge->m_v1 : m_startInfo.m_startEdge->m_v0);
  
  // easiest way to get v2 is to use this function which requires the
  // other indices to already be in the list.
  scratchIndices.push_back(v0);
  scratchIndices.push_back(v1);
  int v2 = NvStripifier::GetNextIndex(scratchIndices, m_startInfo.m_startFace);
  scratchIndices.push_back(v2);

  //
  // build the forward list
  //
  int nv0 = v1;
  int nv1 = v2;

  NvFaceInfo *nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, m_startInfo.m_startFace);
  while (nextFace != NULL && !IsMarked(nextFace)){

    // add this to the strip
    forwardFaces.push_back(nextFace);
    MarkTriangle(nextFace);

    // add the index
    nv0 = nv1;
    nv1 = NvStripifier::GetNextIndex(scratchIndices, nextFace);
    scratchIndices.push_back(nv1);

    // and get the next face
    nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, nextFace);

  }

  //
  // reset the indices for building the strip backwards and do so
  //
  scratchIndices.resize(0);
  scratchIndices.push_back(v2);
  scratchIndices.push_back(v1);
  scratchIndices.push_back(v0);
  nv0 = v1;
  nv1 = v0;
  nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, m_startInfo.m_startFace);
  while (nextFace != NULL && !IsMarked(nextFace)){

    // add this to the strip
    backwardFaces.push_back(nextFace);
    MarkTriangle(nextFace);

    // add the index
    nv0 = nv1;
    nv1 = NvStripifier::GetNextIndex(scratchIndices, nextFace);
    scratchIndices.push_back(nv1);

    // and get the next face
    nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, nextFace);
  }

  // Combine the forward and backwards stripification lists and put into our own face vector
  Combine(forwardFaces, backwardFaces);
}


///////////////////////////////////////////////////////////////////////////////////////////
// Combine()
//
// Combines the two input face vectors and puts the result into m_faces
//
void NvStripInfo::Combine(const NvFaceInfoVec &forward, const NvFaceInfoVec &backward){

  // add backward faces
  int numFaces = backward.size();
  for (int i = numFaces - 1; i >= 0; i--)
    m_faces.push_back(backward[i]);

  // add forward faces
  numFaces = forward.size();
  for (i = 0; i < numFaces; i++)
    m_faces.push_back(forward[i]);
}


///////////////////////////////////////////////////////////////////////////////////////////
// SharesEdge()
//
// Returns true if the input face and the current strip share an edge
//
bool NvStripInfo::SharesEdge(const NvFaceInfo* faceInfo, NvEdgeInfoVec &edgeInfos)
{
  //check v0->v1 edge
  NvEdgeInfo* currEdge = NvStripifier::FindEdgeInfo(edgeInfos, faceInfo->m_v0, faceInfo->m_v1);
  
  if(IsInStrip(currEdge->m_face0) || IsInStrip(currEdge->m_face1))
    return true;

  //check v1->v2 edge
  currEdge = NvStripifier::FindEdgeInfo(edgeInfos, faceInfo->m_v1, faceInfo->m_v2);

  if(IsInStrip(currEdge->m_face0) || IsInStrip(currEdge->m_face1))
    return true;

  //check v2->v0 edge
  currEdge = NvStripifier::FindEdgeInfo(edgeInfos, faceInfo->m_v2, faceInfo->m_v0);
  
  if(IsInStrip(currEdge->m_face0) || IsInStrip(currEdge->m_face1))
    return true;

  return false;
  
}


///////////////////////////////////////////////////////////////////////////////////////////
// CommitStrips()
//
// "Commits" the input strips by setting their m_experimentId to -1 and adding to the allStrips
//  vector
//
void NvStripifier::CommitStrips(NvStripInfoVec &allStrips,
			       const NvStripInfoVec &strips){
  
  // Iterate through strips
  int numStrips = strips.size();
  for (int i = 0; i < numStrips; i++){

    // Tell the strip that it is now real
    NvStripInfo *strip = strips[i];
    strip->m_experimentId = -1;
    
    // add to the list of real strips
    allStrips.push_back(strip);

    // Iterate through the faces of the strip
    // Tell the faces of the strip that they belong to a real strip now
    const NvFaceInfoVec &faces = strips[i]->m_faces;
    int numFaces = faces.size();
    for (int j = 0; j < numFaces; j++)
      strip->MarkTriangle(faces[j]);
  }
}


///////////////////////////////////////////////////////////////////////////////////////////
// FindTraversal()
//
// Finds the next face to start the next strip on.
//
bool NvStripifier::FindTraversal(NvFaceInfoVec &faceInfos,
				NvEdgeInfoVec    &edgeInfos,
				NvStripInfo      *strip,
				NvStripStartInfo &startInfo){
  
  // if the strip was v0->v1 on the edge, then v1 will be a vertex in the next edge.
  int v = (strip->m_startInfo.m_toV1 ? strip->m_startInfo.m_startEdge->m_v1 : strip->m_startInfo.m_startEdge->m_v0);
  
  NvFaceInfo *untouchedFace = NULL;
  NvEdgeInfo *edgeIter      = edgeInfos[v];
  while (edgeIter != NULL){
    NvFaceInfo *face0 = edgeIter->m_face0;
    NvFaceInfo *face1 = edgeIter->m_face1;
    if ((face0 != NULL && !strip->IsInStrip(face0)) && face1 != NULL && !strip->IsMarked(face1))
    {
      untouchedFace = face1;
      break;
    }
    if ((face1 != NULL && !strip->IsInStrip(face1)) && face0 != NULL && !strip->IsMarked(face0)){
      untouchedFace = face0;
      break;
    }

    // find the next edgeIter
    edgeIter = (edgeIter->m_v0 == v ? edgeIter->m_nextV0 : edgeIter->m_nextV1);
  }

  startInfo.m_startFace = untouchedFace;
  startInfo.m_startEdge = edgeIter;
  if (edgeIter != NULL)
  {
    if(strip->SharesEdge(startInfo.m_startFace, edgeInfos))
      startInfo.m_toV1 = (edgeIter->m_v0 == v);  //note! used to be m_v1
    else
      startInfo.m_toV1 = (edgeIter->m_v1 == v);
  }
  return (startInfo.m_startFace != NULL);
}


///////////////////////////////////////////////////////////////////////////////////////////
// Stripify()
//
//
// in_indices are the input indices of the mesh to stripify
// in_cacheSize is the target cache size 
//
void NvStripifier::Stripify(const WordVec &in_indices, const int in_cacheSize, NvStripInfoVec &outStrips) {

  //the number of times to run the experiments
  int numSamples = 10;
  cacheSize = in_cacheSize;

  indices = in_indices;
  
  // build the stripification info
  NvFaceInfoVec allFaceInfos;
  NvEdgeInfoVec allEdgeInfos;
  
  BuildStripifyInfo(allFaceInfos, allEdgeInfos);

  NvStripInfoVec allStrips;
  // stripify
  FindAllStrips(allStrips, allFaceInfos, allEdgeInfos, numSamples);

  //split up the strips into cache friendly pieces, optimize them, then dump these into outStrips
  SplitUpStripsAndOptimize(allStrips, outStrips, allEdgeInfos);

  //clean up
  for(int i = 0; i < allStrips.size(); i++)
  {
    delete allStrips[i];
  }

  for (i = 0; i < allEdgeInfos.size(); i++)
  {
    NvEdgeInfo *info = allEdgeInfos[i];
    while (info != NULL){
      NvEdgeInfo *next = (info->m_v0 == i ? info->m_nextV0 : info->m_nextV1);
      info->Unref();
      info = next;
    }
  }

}


///////////////////////////////////////////////////////////////////////////////////////////
// SplitUpStripsAndOptimize()
//
// Splits the input vector of strips (allStrips) into smaller, cache friendly pieces, then
//  reorders these pieces to maximize cache hits
// The final strips are output through outStrips
//
void NvStripifier::SplitUpStripsAndOptimize(const NvStripInfoVec &allStrips, NvStripInfoVec &outStrips,
                                            NvEdgeInfoVec& edgeInfos)
{
  int threshold = cacheSize - 4;
  NvStripInfoVec tempStrips;

  //split up strips into threshold-sized pieces
  for(int i = 0; i < allStrips.size(); i++)
  {
    NvStripInfo* currentStrip;
    NvStripStartInfo startInfo(NULL, NULL, false);

    if(allStrips[i]->m_faces.size() > threshold)
    {

      int numTimes    = allStrips[i]->m_faces.size() / threshold;
      int numLeftover = allStrips[i]->m_faces.size() % threshold;
      
      for(int j = 0; j < numTimes; j++)
      {
        currentStrip = new NvStripInfo(startInfo, 0, -1);

        for(int faceCtr = j*threshold; faceCtr < threshold+(j*threshold); faceCtr++)
        {
          currentStrip->m_faces.push_back(allStrips[i]->m_faces[faceCtr]);
        }

        tempStrips.push_back(currentStrip);
      }

      int leftOff = j * threshold;

      if(numLeftover != 0)
      {
        currentStrip = new NvStripInfo(startInfo, 0, -1);   

        for(int k = 0; k < numLeftover; k++)
        {
          currentStrip->m_faces.push_back(allStrips[i]->m_faces[leftOff++]);
        }

        tempStrips.push_back(currentStrip);
      }
    }
    else
    {
      //we're not just doing a tempStrips.push_back(allStrips[i]) because
      // this way we can delete allStrips later to free the memory
      currentStrip = new NvStripInfo(startInfo, 0, -1);

      for(int j = 0; j < allStrips[i]->m_faces.size(); j++)
        currentStrip->m_faces.push_back(allStrips[i]->m_faces[j]);

      tempStrips.push_back(currentStrip);
    }
  }


  //Optimize for the vertex cache
  VertexCache* vcache = new VertexCache(cacheSize);
  
  float bestNumHits = -1.0f;
  float numHits;
  int bestIndex;
  bool done = false;

  int firstIndex = 0;
  float minCost = 10000.0f;

  for(i = 0; i < tempStrips.size(); i++)
  {
    int numNeighbors = 0;

    //find strip with least number of neighbors per face
    for(int j = 0; j < tempStrips[i]->m_faces.size(); j++)
    {
      numNeighbors += NumNeighbors(tempStrips[i]->m_faces[j], edgeInfos);
    }

    float currCost = (float)numNeighbors / (float)tempStrips[i]->m_faces.size();
    if(currCost < minCost)
    {
      minCost = currCost;
      firstIndex = i;
    }
  }

  UpdateCache(vcache, tempStrips[firstIndex]);
  outStrips.push_back(tempStrips[firstIndex]);

  tempStrips[firstIndex]->visited = true;

  //this n^2 algo is what slows down stripification so much....
  // needs to be improved
  while(1)
  {
    bestNumHits = -1.0f;

    //find best strip to add next, given the current cache
    for(int i = 0; i < tempStrips.size(); i++)
    {
      if(tempStrips[i]->visited)
        continue;

      numHits = CalcNumHits(vcache, tempStrips[i]);
      if(numHits > bestNumHits)
      {
        bestNumHits = numHits;
        bestIndex = i;
      }
    }

    if(bestNumHits == -1.0f)
      break;
    tempStrips[bestIndex]->visited = true;
    UpdateCache(vcache, tempStrips[bestIndex]);
    outStrips.push_back(tempStrips[bestIndex]);
  }

  delete vcache;

}


///////////////////////////////////////////////////////////////////////////////////////////
// UpdateCache()
//
// Updates the input vertex cache with this strip's vertices
//
void NvStripifier::UpdateCache(VertexCache* vcache, NvStripInfo* strip)
{
  for(int i = 0; i < strip->m_faces.size(); i++)
  {
    if(!vcache->InCache(strip->m_faces[i]->m_v0))
      vcache->AddEntry(strip->m_faces[i]->m_v0);
    
    if(!vcache->InCache(strip->m_faces[i]->m_v1))
      vcache->AddEntry(strip->m_faces[i]->m_v1);
    
    if(!vcache->InCache(strip->m_faces[i]->m_v2))
      vcache->AddEntry(strip->m_faces[i]->m_v2);
  }
}
  

///////////////////////////////////////////////////////////////////////////////////////////
// CalcNumHits()
//
// returns the number of cache hits per face in the strip
//
float NvStripifier::CalcNumHits(VertexCache* vcache, NvStripInfo* strip)
{
  int numHits = 0;
  int numFaces = 0;

  for(int i = 0; i < strip->m_faces.size(); i++)
  {
    if(vcache->InCache(strip->m_faces[i]->m_v0))
      numHits++;
    
    if(vcache->InCache(strip->m_faces[i]->m_v1))
      numHits++;
    
    if(vcache->InCache(strip->m_faces[i]->m_v2))
      numHits++;

    numFaces++;
    
  }

  return ((float)numHits / (float)numFaces);
}


///////////////////////////////////////////////////////////////////////////////////////////
// NumNeighbors()
//
// Returns the number of neighbors that this face has
//
int NvStripifier::NumNeighbors(NvFaceInfo* face, NvEdgeInfoVec& edgeInfoVec)
{
  int numNeighbors = 0;

  if(FindOtherFace(edgeInfoVec, face->m_v0, face->m_v1, face) != NULL)
  {
    numNeighbors++;
  }
  
  if(FindOtherFace(edgeInfoVec, face->m_v1, face->m_v2, face) != NULL)
  {
    numNeighbors++;
  }
  
  if(FindOtherFace(edgeInfoVec, face->m_v2, face->m_v0, face) != NULL)
  {
    numNeighbors++;
  }

  return numNeighbors;
}


///////////////////////////////////////////////////////////////////////////////////////////
// AvgStripSize()
//
// Finds the average strip size of the input vector of strips
//
inline float NvStripifier::AvgStripSize(const NvStripInfoVec &strips){
  int sizeAccum = 0;
  int numStrips = strips.size();
  for (int i = 0; i < numStrips; i++){
    NvStripInfo *strip = strips[i];
    sizeAccum += strip->m_faces.size();
  }
  return ((float)sizeAccum) / ((float)numStrips);
}


///////////////////////////////////////////////////////////////////////////////////////////
// FindAllStrips()
//
// Does the stripification, puts output strips into vector allStrips
//
// Works by setting runnning a number of experiments in different areas of the mesh, and
//  accepting the one which results in the longest strips.  It then accepts this, and moves
//  on to a different area of the mesh.  We try to jump around the mesh some, to ensure that
//  large open spans of strips get generated.
//
void NvStripifier::FindAllStrips(NvStripInfoVec &allStrips,
				NvFaceInfoVec &allFaceInfos,
				NvEdgeInfoVec &allEdgeInfos,
				int numSamples){

  // the experiments
  int experimentId = 0;
  int stripId      = 0;
  bool done        = false;
  
  while (!done){

    //
    // PHASE 1: Set up numSamples * numEdges experiments
    //
    NvStripInfoVec *experiments = new NvStripInfoVec [numSamples * 6];
    int experimentIndex = 0;
    std::set   <NvFaceInfo*>  resetPoints;
    for (int i = 0; i < numSamples; i++){

      // Try to find another good reset point.
      // If there are none to be found, we are done
      NvFaceInfo *nextFace = FindGoodResetPoint(allFaceInfos, allEdgeInfos);
      if (nextFace == NULL){
      	done = true;
      	break;
      }

      // If we have already evaluated starting at this face in this slew
      // of experiments, then skip going any further
      else if (resetPoints.find(nextFace) != resetPoints.end()){
	      continue;
      }

  	  // trying it now...
	    resetPoints.insert(nextFace);

      // otherwise, we shall now try experiments for starting on the 01,12, and 20 edges
      nlassert(nextFace->m_stripId < 0);

      // build the strip off of this face's 0-1 edge
      NvEdgeInfo *edge01 = FindEdgeInfo(allEdgeInfos, nextFace->m_v0, nextFace->m_v1);
      NvStripInfo *strip01 = new NvStripInfo(NvStripStartInfo(nextFace, edge01, true), stripId++, experimentId++);
      experiments[experimentIndex++].push_back(strip01);

      // build the strip off of this face's 1-0 edge
      NvEdgeInfo *edge10 = FindEdgeInfo(allEdgeInfos, nextFace->m_v0, nextFace->m_v1);
      NvStripInfo *strip10 = new NvStripInfo(NvStripStartInfo(nextFace, edge10, false), stripId++, experimentId++);
      experiments[experimentIndex++].push_back(strip10);
      
      // build the strip off of this face's 1-2 edge
      NvEdgeInfo *edge12 = FindEdgeInfo(allEdgeInfos, nextFace->m_v1, nextFace->m_v2);
      NvStripInfo *strip12 = new NvStripInfo(NvStripStartInfo(nextFace, edge12, true), stripId++, experimentId++);
      experiments[experimentIndex++].push_back(strip12);

      // build the strip off of this face's 2-1 edge
      NvEdgeInfo *edge21 = FindEdgeInfo(allEdgeInfos, nextFace->m_v1, nextFace->m_v2);
      NvStripInfo *strip21 = new NvStripInfo(NvStripStartInfo(nextFace, edge21, false), stripId++, experimentId++);
      experiments[experimentIndex++].push_back(strip21);

      // build the strip off of this face's 2-0 edge
      NvEdgeInfo *edge20 = FindEdgeInfo(allEdgeInfos, nextFace->m_v2, nextFace->m_v0);
      NvStripInfo *strip20 = new NvStripInfo(NvStripStartInfo(nextFace, edge20, true), stripId++, experimentId++);
      experiments[experimentIndex++].push_back(strip20);

      // build the strip off of this face's 0-2 edge
      NvEdgeInfo *edge02 = FindEdgeInfo(allEdgeInfos, nextFace->m_v2, nextFace->m_v0);
      NvStripInfo *strip02 = new NvStripInfo(NvStripStartInfo(nextFace, edge02, false), stripId++, experimentId++);
      experiments[experimentIndex++].push_back(strip02);
    }

    //
    // PHASE 2: Iterate through that we setup in the last phase
    // and really build each of the strips and strips that follow to see how
    // far we get
    //
    int numExperiments = experimentIndex;
    for (i = 0; i < numExperiments; i++){

      // get the strip set
      
      // build the first strip of the list
      experiments[i][0]->Build(allEdgeInfos, allFaceInfos);
      int experimentId = experiments[i][0]->m_experimentId;

      NvStripInfo *stripIter = experiments[i][0];
      NvStripStartInfo startInfo(NULL, NULL, false);
      while (FindTraversal(allFaceInfos, allEdgeInfos, stripIter, startInfo)){
	
      	// create the new strip info
        stripIter = new NvStripInfo(startInfo, stripId++, experimentId);

	      // build the next strip
	      stripIter->Build(allEdgeInfos, allFaceInfos);

	      // add it to the list
	      experiments[i].push_back(stripIter);
      }
    }

    //
    // Phase 3: Find the experiment that has the most promise
    //
    int bestIndex = 0;
    float bestValue = 0;
	  for (i = 0; i < numExperiments; i++){
      const float avgStripSizeWeight = 1.0f;
      const float numTrisWeight      = 1.0f;
      float avgStripSize = AvgStripSize(experiments[i]);
      float numStrips    = (float) experiments[i].size();
      float value        = avgStripSize * avgStripSizeWeight + (avgStripSize * numStrips * numTrisWeight);
      
      //float value = CalculateValue(vcache, experiments[i]);
      if (value > bestValue){
	      bestValue = value;
	      bestIndex = i;
      }
    }

    //
    // Phase 4: commit the best experiment of the bunch
    //
    CommitStrips(allStrips, experiments[bestIndex]);

    // and destroy all of the others
	  for (i = 0; i < numExperiments; i++)
    {
      if (i != bestIndex)
      {
  		  int numStrips = experiments[i].size();
		    for (int j = 0; j < numStrips; j++)
        {
          delete experiments[i][j];
        }
      }
	  }
    
    // delete the array that we used for all experiments
    delete [] experiments;
  }
}


///////////////////////////////////////////////////////////////////////////////////////////
// CountRemainingTris()
//
// This will count the number of triangles left in the
// strip list starting at iter and finishing up at end
//
inline int NvStripifier::CountRemainingTris(std::list<NvStripInfo*>::iterator iter,
				     std::list<NvStripInfo*>::iterator  end){
  int count = 0;
  while (iter != end){
    count += (*iter)->m_faces.size();
    iter++;
  }
  return count;
}

