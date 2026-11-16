-- База для учета отпусков
-- Запускать в SQL Server Management Studio

IF DB_ID(N'OtpuskKompanii') IS NOT NULL
BEGIN
    ALTER DATABASE OtpuskKompanii SET SINGLE_USER WITH ROLLBACK IMMEDIATE;
    DROP DATABASE OtpuskKompanii;
END
GO

CREATE DATABASE OtpuskKompanii;
GO

USE OtpuskKompanii;
GO

-- отделы
CREATE TABLE Otdely (
    Id INT NOT NULL PRIMARY KEY,
    Nazvanie NVARCHAR(50) NOT NULL
);
GO

-- сотрудники
CREATE TABLE Sotrudniki (
    Id INT NOT NULL PRIMARY KEY,
    Fio NVARCHAR(80) NOT NULL,
    Tabel NVARCHAR(20) NOT NULL,
    OtdelId INT NOT NULL,
    Norma INT NOT NULL,
    CONSTRAINT FK_Sotr_Otdel FOREIGN KEY (OtdelId) REFERENCES Otdely(Id)
);
GO

-- отпуска
-- даты записаны текстом, дни считает программа
CREATE TABLE Otpuska (
    Id INT NOT NULL PRIMARY KEY,
    SotrId INT NOT NULL,
    DataNachala NVARCHAR(20) NOT NULL,
    DataKonca NVARCHAR(20) NOT NULL,
    Dney INT NOT NULL,
    CONSTRAINT FK_Otp_Sotr FOREIGN KEY (SotrId) REFERENCES Sotrudniki(Id)
);
GO
