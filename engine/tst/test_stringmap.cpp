#include "gtest/gtest.h"
#include "data_structures.h"
#include <vector>
#include <memory>

namespace EngineTests
{
    using namespace SketchBook::DataStructures;

    TEST(StringMapTests, BasicUsage)
    {
        struct BaseStruct { 
            std::vector<std::string> data{}; 
            bool operator==(BaseStruct& other) { (data.size() == other.data.size()) && (data == other.data); }
        };

        struct DerivedStruct : public BaseStruct { 
            std::vector<float> numbers{}; 
        };

        typedef StringMap<BaseStruct> StructMap;

        StructMap struct_map{};

        auto* a = struct_map.add_elem<BaseStruct>("a");
        a->data.push_back("hello");
        EXPECT_TRUE(a->data[0] == struct_map.get_elem<BaseStruct>("a")->data[0]);

        auto* b = struct_map.add_elem_with_lambda<DerivedStruct>("b", [](DerivedStruct*& elem){
            elem->numbers.push_back(5);
        });

        DerivedStruct *c_val = nullptr;
        DerivedStruct *d_val = nullptr;
        DerivedStruct* c = nullptr;
        DerivedStruct d {{{"foo", "bar"}}, {324.f,3432.f,2.f}};
        EXPECT_TRUE(b->numbers[0] == struct_map.get_elem<DerivedStruct>("b")->numbers[0]);
        {
            c  = new DerivedStruct{{{"foo", "bar"}}, {324.f,3432.f,2.f}};
            c_val = struct_map.add_elem_with_lambda<DerivedStruct>("c_val", [=](DerivedStruct*& elem){
                *elem = *c;
                // second below assertion should fail for:
                // elem = c;
            });

            ASSERT_FALSE(c == c_val);
            c->numbers[0] = 0.f;

            ASSERT_FALSE(c->numbers[0] == c_val->numbers[0]);

            d_val = struct_map.add_elem_with_lambda<DerivedStruct>("d_val", [=](DerivedStruct*& elem){
                elem->data = d.data;
                elem->numbers = d.numbers;
            });

            ASSERT_NO_THROW(c_val->data.size());
        }

        ASSERT_NO_THROW(c_val->data.size());
        ASSERT_NO_THROW(d_val->data.size() == 2);
        c = nullptr;
        ASSERT_NO_THROW(c_val->data.size());
        return;
    }

}