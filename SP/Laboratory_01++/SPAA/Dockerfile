FROM gcc:latest

WORKDIR /app

COPY SP_AA/SP_AA.cpp SP_04/SP_04.cpp ./

# Компилируем .so
RUN g++ -shared -fPIC SP_AA.cpp -o libSP_AA.so

# Компилируем приложение
RUN g++ SP_04.cpp -L. -lSP_AA -o SP_04

ENV LD_LIBRARY_PATH=/app

# Демонстрация
CMD ["./SP_04"]

# Создание изображения docker build -t sp03_app .
# Запуск docker run --rm -it sp04_app
# Удаление docker rmi sp04_app