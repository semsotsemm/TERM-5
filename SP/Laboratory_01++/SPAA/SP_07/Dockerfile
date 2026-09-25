FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
WORKDIR /app

RUN apt-get update && apt-get install -y g++

COPY SP_07/sp_aa.cpp ./
RUN g++ -shared -fPIC sp_aa.cpp -o libSP_AA.so

RUN dotnet new console -n SP_07_App
WORKDIR /app/SP_07_App

COPY SP_07/Program.cs ./Program.cs

RUN cp /app/libSP_AA.so bin/Release/net8.0/ || cp /app/libSP_AA.so bin/Debug/net8.0/

RUN dotnet publish -c Release -o /app/out

FROM mcr.microsoft.com/dotnet/runtime:8.0
WORKDIR /app
COPY --from=build /app/out ./
COPY --from=build /app/libSP_AA.so ./

ENV LD_LIBRARY_PATH=/app

CMD ["dotnet", "SP_07_App.dll"]

# Создание контейнера docker build -t sp07_app .
# Запуск docker run --rm -it sp07_app
# Удаление docker rmi -f sp07_app