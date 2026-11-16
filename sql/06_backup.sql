-- Резервная копия. Путь можно поменять на свою папку.
-- У учетной записи службы SQL Server должны быть права на эту папку.
BACKUP DATABASE OtpuskKompanii
TO DISK = N'C:\Temp\OtpuskKompanii.bak'
WITH INIT, NAME = N'OtpuskKompanii';
GO
