# The media broker belongs to the interactive user. The encoder runs in session 0.
$ErrorActionPreference = 'Stop'
$action = New-ScheduledTaskAction -Execute "$env:ProgramFiles\LdacA2dp\LdacMedia.exe"
$trigger = New-ScheduledTaskTrigger -AtLogOn
$principal = New-ScheduledTaskPrincipal -GroupId 'S-1-5-4' -RunLevel Limited
$settings = New-ScheduledTaskSettingsSet -MultipleInstances Parallel -ExecutionTimeLimit ([TimeSpan]::Zero) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries
Register-ScheduledTask -TaskName 'LdacA2dp-Media' -Action $action -Trigger $trigger -Principal $principal -Settings $settings -Force | Out-Null
Start-ScheduledTask -TaskName 'LdacA2dp-Media'
