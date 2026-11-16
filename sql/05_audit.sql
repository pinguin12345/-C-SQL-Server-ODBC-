-- Короткий журнал изменений отпусков внутри SQL Server
USE OtpuskKompanii;
GO

IF OBJECT_ID(N'Zhurnal', N'U') IS NULL
CREATE TABLE Zhurnal (
    Id INT IDENTITY PRIMARY KEY,
    Kogda DATETIME NOT NULL DEFAULT GETDATE(),
    Chto NVARCHAR(40) NOT NULL
);
GO

CREATE OR ALTER TRIGGER tr_otpusk_zhurnal ON Otpuska
AFTER INSERT, DELETE
AS
BEGIN
    SET NOCOUNT ON;
    INSERT INTO Zhurnal (Chto)
    SELECT N'добавлен отпуск' FROM inserted;
    INSERT INTO Zhurnal (Chto)
    SELECT N'удален отпуск' FROM deleted;
END;
GO
