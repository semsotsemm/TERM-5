FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
WORKDIR /app

# Устанавливаем g++ для компиляции C++ части
RUN apt-get update && apt-get install -y g++

# Копируем исходник C++ и собираем libSP_XX.so
COPY SP_08/sp_xx.cpp ./
RUN g++ -shared -fPIC sp_xx.cpp -o libSP_XX.so

# Создаем проект C#
RUN dotnet new console -n SP_08_App
WORKDIR /app/SP_08_App

# Копируем наш файл Program.cs с динамическим вызовом через dlopen/dlsym
COPY SP_08/Program.cs ./Program.cs

# Переносим скомпилированную .so библиотеку в каталог сборки
RUN cp /app/libSP_XX.so bin/Release/net8.0/ || cp /app/libSP_XX.so bin/Debug/net8.0/

# Публикуем приложение
RUN dotnet publish -c Release -o /app/out

# Финальный образ
FROM mcr.microsoft.com/dotnet/runtime:8.0
WORKDIR /app
COPY --from=build /app/out ./
COPY --from=build /app/libSP_XX.so ./

ENV LD_LIBRARY_PATH=/app

CMD ["dotnet", "SP_08_App.dll"]