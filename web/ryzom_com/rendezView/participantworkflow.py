# 
#
# Generated by dumpDCWorkflow.py written by Sebastien Bigaret
# Original workflow id/title: participant_workflow/Participant Workflow
# Date: 2006/08/17 12:54:10.878 GMT+2
#
# WARNING: this dumps does NOT contain any scripts you might have added to
# the workflow, IT IS YOUR RESPONSABILITY TO MAKE BACKUPS FOR THESE SCRIPTS.
#
# No script detected in this workflow
# 
"""
Programmatically creates a workflow type
"""
__version__ = "$Revision: 1.1 $"[11:-2]

from Products.CMFCore.WorkflowTool import addWorkflowFactory

from Products.DCWorkflow.DCWorkflow import DCWorkflowDefinition

def setupParticipant_workflow(wf):
    "..."
    wf.setProperties(title='Participant Workflow')

    for s in ['public']:
        wf.states.addState(s)
    for t in []:
        wf.transitions.addTransition(t)
    for v in []:
        wf.variables.addVariable(v)
    for l in []:
        wf.worklists.addWorklist(l)
    for p in ('Access contents information', 'Change portal events', 'Modify portal content', 'View'):
        wf.addManagedPermission(p)
        

    ## Initial State
    wf.states.setInitialState('public')

    ## States initialization
    sdef = wf.states['public']
    sdef.setProperties(title="""Public""",
                       transitions=())
    sdef.setPermission('Access contents information', 1, ['Anonymous', 'Authenticated', 'Manager', 'Member', 'Owner', 'Reviewer'])
    sdef.setPermission('Change portal events', 1, ['Manager', 'Owner'])
    sdef.setPermission('Modify portal content', 1, ['Manager', 'Owner'])
    sdef.setPermission('View', 1, ['Anonymous', 'Authenticated', 'Manager', 'Member', 'Owner', 'Reviewer'])


    ## Transitions initialization
    ## State Variable
    wf.variables.setStateVar('state')

    ## Variables initialization
    ## Worklists Initialization

def createParticipant_workflow(id):
    "..."
    ob = DCWorkflowDefinition(id)
    setupParticipant_workflow(ob)
    return ob

addWorkflowFactory(createParticipant_workflow,
                   id='participant_workflow',
                   title='Participant Workflow')

    