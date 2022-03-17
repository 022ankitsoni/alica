package de.unikassel.vs.alica.planDesigner.alicamodel;

import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.beans.property.SimpleObjectProperty;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Plan extends AbstractPlan {

    protected final SimpleBooleanProperty masterPlan = new SimpleBooleanProperty();
    protected final SimpleDoubleProperty utilityThreshold = new SimpleDoubleProperty();
    protected final SimpleObjectProperty<PreCondition> preCondition = new SimpleObjectProperty<>();
    protected final SimpleObjectProperty<RuntimeCondition> runtimeCondition = new SimpleObjectProperty<>();

    protected final ArrayList<EntryPoint> entryPoints = new ArrayList<>();
    protected final ArrayList<State> states = new ArrayList<>();
    protected final ArrayList<Transition> transitions = new ArrayList<>();
    protected final ArrayList<Synchronisation> synchronisations = new ArrayList<>();

    /**
     * Need to be explicitly written, because the constructor Plan(long id)
     * makes the autogenerated version of this one vanishing.
     */
    public Plan() {
        super();
    }

    /**
     * Constructor for representing incomplete Plan References while parsing files.
     * @param id The ID found in the file, while parsing the reference.
     */
    public Plan(long id) {
        this();
        this.id = id ;
    }

    public boolean getMasterPlan() {
        return masterPlan.get();
    }
    public void setMasterPlan(boolean masterPlan) {
        this.masterPlan.set(masterPlan);
    }
    public SimpleBooleanProperty masterPlanProperty() {
        return masterPlan;
    }

    public double getUtilityThreshold() {
        return utilityThreshold.get();
    }
    public void setUtilityThreshold(double utilityThreshold) {
        this.utilityThreshold.set(utilityThreshold);
    }
    public SimpleDoubleProperty utilityThreshold() {
        return utilityThreshold;
    }

    public PreCondition getPreCondition() {
        return preCondition.get();
    }
    public void setPreCondition(PreCondition preCondition) {
        this.preCondition.set(preCondition);
        if(preCondition != null) {
            preCondition.registerDirtyFlag(this.changeListenerForDirtyFlag);
        }
    }
    public SimpleObjectProperty<PreCondition> preConditionProperty(){
        return preCondition;
    }

    public RuntimeCondition getRuntimeCondition() {
        return runtimeCondition.get();
    }
    public void setRuntimeCondition(RuntimeCondition runtimeCondition) {
        this.runtimeCondition.set(runtimeCondition);
        if(runtimeCondition != null) {
            runtimeCondition.registerDirtyFlag(this.changeListenerForDirtyFlag);
        }
    }
    public SimpleObjectProperty<RuntimeCondition> runtimeConditionProperty(){
        return runtimeCondition;
    }

    public void addTransition(Transition transition) {
        transitions.add(transition);
        transition.registerDirtyFlag(this.changeListenerForDirtyFlag);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public void removeTransition(Transition transition) {
        transitions.remove(transition);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public List<Transition> getTransitions() {
        return Collections.unmodifiableList(transitions);
    }

    public void addState(State state) {
        states.add(state);
        state.registerDirtyFlag(this.changeListenerForDirtyFlag);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public void removeState(State state) {
        states.remove(state);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public List<State> getStates() {
        return Collections.unmodifiableList(states);
    }

    public void addSynchronisation(Synchronisation synchronisation) {
        synchronisations.add(synchronisation);
        synchronisation.registerDirtyFlag(this.changeListenerForDirtyFlag);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public void removeSynchronisation(Synchronisation synchronisation) {
        synchronisations.remove(synchronisation);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public List<Synchronisation> getSynchronisations() {
        return Collections.unmodifiableList(synchronisations);
    }

    public void addEntryPoint(EntryPoint entryPoint) {
        entryPoints.add(entryPoint);
        entryPoint.registerDirtyFlag(this.changeListenerForDirtyFlag);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public void removeEntryPoint(EntryPoint entryPoint) {
        entryPoints.remove(entryPoint);
        this.changeListenerForDirtyFlag.setDirty();
    }
    public List<EntryPoint> getEntryPoints() {
        return Collections.unmodifiableList(entryPoints);
    }

    @Override
    public void registerDirtyFlag() {
        super.registerDirtyFlag();
        this.masterPlan.addListener(this.changeListenerForDirtyFlag);
        this.utilityThreshold.addListener(this.changeListenerForDirtyFlag);
        this.preCondition.addListener(this.changeListenerForDirtyFlag);
        this.runtimeCondition.addListener(this.changeListenerForDirtyFlag);

        for (EntryPoint ep : this.entryPoints) {
            ep.registerDirtyFlag(this.changeListenerForDirtyFlag);
        }

        for (State state : states) {
            state.registerDirtyFlag(this.changeListenerForDirtyFlag);
        }

        for (Transition transition : transitions) {
            transition.registerDirtyFlag(this.changeListenerForDirtyFlag);
        }

        for (Synchronisation synchronisation : synchronisations) {
            synchronisation.registerDirtyFlag(this.changeListenerForDirtyFlag);
        }

        this.setDirty(false);
    }

    @Override
    public void removeVariable(Variable variable) {
        super.removeVariable(variable);

        // Remove the Variable from all child-elements
        if(getPreCondition() != null) {
            getPreCondition().removeVariable(variable);
        }
        if(getRuntimeCondition() != null) {
            getRuntimeCondition().removeVariable(variable);
        }
        for(State state : getStates()) {
            if(state instanceof TerminalState && ((TerminalState) state).getPostCondition() != null) {
                ((TerminalState) state).getPostCondition().removeVariable(variable);
            }
        }
        for(Transition transition : getTransitions()) {
            if(transition.getPreCondition() != null) {
                transition.getPreCondition().removeVariable(variable);
            }
        }
    }
}
