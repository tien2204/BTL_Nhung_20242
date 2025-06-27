#ifndef MODEL_HPP
#define MODEL_HPP

#include <cstdint>

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    /** Lưu điểm cao nhất (hoặc lastScore) vào Model */
    void setLastScore(uint16_t s) { lastScore = s; }

    /** Trả về điểm cao nhất (hoặc lastScore) */
    uint16_t getLastScore() const  { return lastScore; }

    void tick();
protected:
    ModelListener* modelListener;

private:
    // biến thành viên để giữ lastScore
    uint16_t lastScore = 0;

};


#endif // MODEL_HPP
