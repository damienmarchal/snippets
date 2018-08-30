#include <SceneCreator/SceneCreator.h>
#include <SofaTest/Sofa_test.h>

#include <SofaSimulationGraph/DAGSimulation.h>
#include <sofa/core/objectmodel/DDGNode.h>
#include <sofa/simulation/DefaultAnimationLoop.h>
#include <sofa/simulation/Simulation.h>

using sofa::core::objectmodel::BaseObject;
using sofa::simulation::DefaultAnimationLoop;
using sofa::simulation::Node;
using sofa::simulation::Simulation;

#include <SofaSimulationCommon/SceneLoaderXML.h>
using sofa::core::ExecParams;
using sofa::simulation::SceneLoaderXML;
#include <sofa/simulation/AnimateVisitor.h>
#include <sofa/simulation/UpdateContextVisitor.h>
#include <sofa/simulation/UpdateMappingVisitor.h>
#include <sofa/simulation/PropagateEventVisitor.h>
#include <sofa/simulation/AnimateBeginEvent.h>

namespace sofa
{
struct Increment : public core::objectmodel::BaseObject
{
public:
    SOFA_CLASS(Increment, sofa::core::objectmodel::BaseObject);

    sofa::Data<int> d_a;
    sofa::Data<int> d_b;
    sofa::Data<int> d_c;

    Increment()
        : d_a(initData(&d_a, 0, "a", "")),
          d_b(initData(&d_b, 0, "b", "")),
          d_c(initData(&d_c, 0, "c", ""))
    {
        f_listening.setValue(true);
    }

    void init() override
    {
        std::cout << getClassName() << "::init()" << std::endl;
    }

    void handleEvent(sofa::core::objectmodel::Event* e) override
    {
        if (sofa::simulation::AnimateBeginEvent::checkEventType(e))
        {
            std::cout << getClassName() << "::HandleEvent()" << std::endl;
            d_a.setValue(d_a.getValue() + 1);
            d_b.setValue(d_b.getValue() + 1);
            d_c.setValue(d_c.getValue() + 1);
        }
    }
};
struct Dump : public Increment
{
public:
    SOFA_CLASS(Dump, Increment);

    Dump() : Increment()
    {
        f_listening.setValue(true);
    }

    void init() override
    {
        std::cout << getClassName() << "::init()" << std::endl;
    }

    void handleEvent(sofa::core::objectmodel::Event* e) override
    {
        if (sofa::simulation::AnimateBeginEvent::checkEventType(e))
        {
            std::cout << getClassName() << "::HandleEvent()" << std::endl;
            std::cout << d_a.getValue() << std::endl;
            std::cout << d_b.getValue() << std::endl;
            std::cout << d_c.getValue() << std::endl << std::endl;
        }
    }
};
struct DefaultImpl : public sofa::core::DataEngine
{
public:
    sofa::Data<int> d_a;
    sofa::Data<int> d_b;
    sofa::Data<int> d_c;
    sofa::Data<int> d_a_out;
    sofa::Data<int> d_b_out;
    sofa::Data<int> d_c_out;

    SOFA_CLASS(DefaultImpl, sofa::core::DataEngine);

    DefaultImpl()
        : d_a(initData(&d_a, 0, "a", "")),
          d_b(initData(&d_b, 0, "b", "")),
          d_c(initData(&d_c, 0, "c", "")),
          d_a_out(initData(&d_a_out, "a_out", "")),
          d_b_out(initData(&d_b_out, "b_out", "")),
          d_c_out(initData(&d_c_out, "c_out", ""))
    {
        f_listening.setValue(true);
    }

    virtual void init() override
    {
        std::cout << getClassName() << "::" << getName() << "::init()" << std::endl;
        addInput(&d_a);
        addInput(&d_b);
        addInput(&d_c);

        addOutput(&d_a_out);
        addOutput(&d_b_out);
        addOutput(&d_c_out);

//        d_a_out.setValue(d_a.getValue());
//        d_b_out.setValue(d_b.getValue());
//        d_c_out.setValue(d_c.getValue());
        setDirtyValue();
    }

    virtual void update() override
    {
        std::cout << getClassName() << "::" << getName() << "::update()" << std::endl;
        d_a.getValue();
        d_b.getValue();
        d_c.getValue();

        this->cleanDirty();

        d_a_out.setValue(d_a.getValue() + 1);
        d_b_out.setValue(d_b.getValue() + 1);
        d_c_out.setValue(d_c.getValue() + 1);
    }
};
struct NoCleanDirty : public DefaultImpl
{
    SOFA_CLASS(NoCleanDirty, DefaultImpl);

    virtual void update() override
    {
        std::cout << getClassName() << "::" << getName() << "::update()" << std::endl;
        d_a_out.setValue(d_a.getValue() + 1);
        d_b_out.setValue(d_b.getValue() + 1);
        d_c_out.setValue(d_c.getValue() + 1);
    }
};
struct OutAccessFirst : public DefaultImpl
{
    SOFA_CLASS(OutAccessFirst, DefaultImpl);

    virtual void update() override
    {
        std::cout << getClassName() << "::" << getName() << "::update()" << std::endl;
        d_a_out.setValue(d_a_out.getValue());
        d_b_out.setValue(d_a_out.getValue());
        d_c_out.setValue(d_a_out.getValue());

        d_a_out.setValue(d_a.getValue() + 1);
        d_b_out.setValue(d_b.getValue() + 1);
        d_c_out.setValue(d_c.getValue() + 1);
    }
};
struct ToggleInputs : public DefaultImpl
{
    SOFA_CLASS(ToggleInputs, DefaultImpl);

    virtual void update() override
    {
        std::cout << getClassName() << "::" << getName() << "::update()" << std::endl;
        if (d_a.getValue() % 2 == 0)
        {
            this->delInput(&d_b);
            this->delInput(&d_c);
        }
        else
        {
            this->addInput(&d_b);
            this->addInput(&d_c);
        }
        d_b.getValue();
        d_c.getValue();

        d_a_out.setValue(d_a.getValue() + 1);
        d_b_out.setValue(d_b.getValue() + 1);
        d_c_out.setValue(d_c.getValue() + 1);
    }
};


struct DataEngine_test : public sofa::BaseTest
{
    simulation::Simulation* simu;
    sofa::simulation::Node::SPtr root;
    sofa::Increment::SPtr input;
    sofa::core::DataEngine::SPtr engine;
    sofa::core::DataEngine::SPtr engine2;
    sofa::Dump::SPtr output;
    core::objectmodel::BaseObjectDescription desc;

    DataEngine_test() {}

    template <class T>
    void testImpl(T* /*ptr*/ = nullptr)
    {
        try {
            using modeling::addNew;
            sofa::simulation::setSimulation(
                        simu = new sofa::simulation::graph::DAGSimulation());
            root = simu->createNewGraph("root");
            root->setDt(0.1);
            root->setGravity( sofa::defaulttype::Vec3d(0,0,0) );
            root->addObject(core::objectmodel::New<simulation::DefaultAnimationLoop>());
            input = addNew<Increment>(root);
            engine = addNew<T>(root);
            engine->setName("1");
            engine2 = addNew<T>(root);
            engine2->setName("2");
            output = addNew<Dump>(root);

            desc = core::objectmodel::BaseObjectDescription("Increment");
            desc.setAttribute("name", "input");
            desc.setAttribute("a", "1");
            desc.setAttribute("b", "2");
            desc.setAttribute("c", "3");
            input->parse(&desc);

            desc = core::objectmodel::BaseObjectDescription("DataEngine");
            desc.setAttribute("name", "1");
            desc.setAttribute("a", "@input.a");
            desc.setAttribute("b", "@input.b");
            desc.setAttribute("c", "@input.c");
            engine->parse(&desc);

            desc = core::objectmodel::BaseObjectDescription("DataEngine");
            desc.setAttribute("name", "2");
            desc.setAttribute("a", "@1.a_out");
            desc.setAttribute("b", "@1.b_out");
            desc.setAttribute("c", "@1.c_out");
            engine2->parse(&desc);

            desc = core::objectmodel::BaseObjectDescription("Dump");
            desc.setAttribute("name", "output");
            desc.setAttribute("a", "@2.a_out");
            desc.setAttribute("b", "@2.b_out");
            desc.setAttribute("c", "@2.c_out");
            output->parse(&desc);

            std::cout << "Init:" << std::endl;
            root->init(sofa::core::ExecParams::defaultInstance());
            std::cout << "Step:" << std::endl;
            simu->animate(root.get(),0.1);
            std::cout << "Step:" << std::endl;
            simu->animate(root.get(),0.1);
            std::cout << "Step:" << std::endl;
            simu->animate(root.get(),0.1);
            simu->unload(root);
        } catch (std::exception& e ) {
            std::cout << e.what() << std::endl;
        }
    }


    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

// this is what is recommended according to comments in DataEngine. Does not work: cyclic recursion on 2::update(). Why?
TEST_F(DataEngine_test, testDefaultImpl)
{
    this->testImpl<DefaultImpl>();
}
// a = b = c = 3   << Why?
TEST_F(DataEngine_test, testNoCleanDirty)
{
    this->testImpl<NoCleanDirty>();
}
// a, b, c = 3, 4, 5   << Works. Yet cleanDirty is not called, and outputs are accessed first. wtf?
TEST_F(DataEngine_test, testOutFirst)
{
    this->testImpl<OutAccessFirst>();
}
// same as testNoCleanDirty. Why at least a should trigger the call to update()
TEST_F(DataEngine_test, testToggleIO)
{
    this->testImpl<ToggleInputs>();
}

}  // namespace sofa
