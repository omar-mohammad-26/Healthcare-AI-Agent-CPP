param(
    [string]$BaseUrl = "http://localhost:8000"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$script:PassedTests = 0
$script:FailedTests = 0

function Write-PassedTest {
    param(
        [Parameter(Mandatory)]
        [string]$Name
    )

    $script:PassedTests++

    Write-Host "[PASS] $Name" -ForegroundColor Green
}

function Write-FailedTest {
    param(
        [Parameter(Mandatory)]
        [string]$Name,

        [Parameter(Mandatory)]
        [string]$Message
    )

    $script:FailedTests++

    Write-Host "[FAIL] $Name" -ForegroundColor Red
    Write-Host "       $Message" -ForegroundColor Red
}

function Invoke-SmokeRequest {
    param(
        [Parameter(Mandatory)]
        [string]$Name,

        [Parameter(Mandatory)]
        [string]$Method,

        [Parameter(Mandatory)]
        [string]$Path,

        [int]$ExpectedStatusCode = 200,

        [hashtable]$Headers = @{},

        [scriptblock]$ValidateResponse
    )

    $requestUri =
        "$($BaseUrl.TrimEnd('/'))$Path"

    try {
        $response = Invoke-WebRequest `
            -UseBasicParsing `
            -Method $Method `
            -Uri $requestUri `
            -Headers $Headers

        if ($response.StatusCode -ne $ExpectedStatusCode) {
            Write-FailedTest `
                -Name $Name `
                -Message (
                    "Expected HTTP $ExpectedStatusCode but received " +
                    "$($response.StatusCode)."
                )

            return
        }

        if ($ValidateResponse) {
            & $ValidateResponse $response
        }

        Write-PassedTest -Name $Name
    }
    catch {
        Write-FailedTest `
            -Name $Name `
            -Message $_.Exception.Message
    }
}

function Convert-ResponseJson {
    param(
        [Parameter(Mandatory)]
        $Response
    )

    if ([string]::IsNullOrWhiteSpace($Response.Content)) {
        throw "The endpoint returned an empty response body."
    }

    try {
        return $Response.Content | ConvertFrom-Json
    }
    catch {
        throw "The endpoint returned invalid JSON."
    }
}

Write-Host ""
Write-Host "Healthcare AI Agent API Smoke Tests" -ForegroundColor Cyan
Write-Host "Backend: $BaseUrl" -ForegroundColor DarkGray
Write-Host ""

Invoke-SmokeRequest `
    -Name "Backend health endpoint" `
    -Method "GET" `
    -Path "/health" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($body.status -ne "ok") {
            throw "Expected health status 'ok'."
        }

        if ($body.service -ne "healthcare-ai-agent") {
            throw "Unexpected backend service name."
        }
    }

Invoke-SmokeRequest `
    -Name "Database health endpoint" `
    -Method "GET" `
    -Path "/health/database" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if (-not $body.status) {
            throw "The database health response has no status field."
        }
    }

Invoke-SmokeRequest `
    -Name "Department listing endpoint" `
    -Method "GET" `
    -Path "/departments" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($null -eq $body.data) {
            throw "The departments response has no data field."
        }
    }

Invoke-SmokeRequest `
    -Name "Doctor listing endpoint" `
    -Method "GET" `
    -Path "/doctors" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($null -eq $body.data) {
            throw "The doctors response has no data field."
        }
    }

Invoke-SmokeRequest `
    -Name "Patient listing endpoint" `
    -Method "GET" `
    -Path "/patients" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($null -eq $body.data) {
            throw "The patients response has no data field."
        }
    }

Invoke-SmokeRequest `
    -Name "Appointment listing endpoint" `
    -Method "GET" `
    -Path "/appointments" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($null -eq $body.data) {
            throw "The appointments response has no data field."
        }
    }

Invoke-SmokeRequest `
    -Name "Treatment listing endpoint" `
    -Method "GET" `
    -Path "/treatments" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($null -eq $body.data) {
            throw "The treatments response has no data field."
        }
    }

Invoke-SmokeRequest `
    -Name "Agent tool registry endpoint" `
    -Method "GET" `
    -Path "/agent/tools" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($body.count -ne 8) {
            throw "Expected exactly 8 registered Agent tools."
        }

        if ($null -eq $body.data) {
            throw "The Agent tool response has no data field."
        }
    }

Invoke-SmokeRequest `
    -Name "Gemini tool declarations endpoint" `
    -Method "GET" `
    -Path "/agent/gemini-tools" `
    -ValidateResponse {
        param($Response)

        $body = Convert-ResponseJson -Response $Response

        if ($body.count -ne 8) {
            throw "Expected exactly 8 Gemini tool declarations."
        }

        if ($null -eq $body.data -or $body.data.Count -lt 1) {
            throw "The Gemini tool response has no declaration group."
        }

        $declarations =
            $body.data[0].functionDeclarations

        if ($null -eq $declarations) {
            throw "The response has no functionDeclarations field."
        }

        if ($declarations.Count -ne 8) {
            throw "Expected exactly 8 function declarations."
        }
    }

Invoke-SmokeRequest `
    -Name "Frontend CORS preflight" `
    -Method "OPTIONS" `
    -Path "/agent/run" `
    -ExpectedStatusCode 204 `
    -Headers @{
        Origin = "http://localhost:3000"
        "Access-Control-Request-Method" = "POST"
        "Access-Control-Request-Headers" = "content-type"
    } `
    -ValidateResponse {
        param($Response)

        $allowedOrigin =
            $Response.Headers["access-control-allow-origin"]

        if ($allowedOrigin -ne "http://localhost:3000") {
            throw "The expected frontend CORS origin was not returned."
        }

        $allowedMethods =
            $Response.Headers["access-control-allow-methods"]

        if ($allowedMethods -notmatch "POST") {
            throw "POST is missing from the allowed CORS methods."
        }
    }

Write-Host ""
Write-Host "Passed: $script:PassedTests" -ForegroundColor Green
Write-Host "Failed: $script:FailedTests" -ForegroundColor Red
Write-Host ""

if ($script:FailedTests -gt 0) {
    exit 1
}

Write-Host "All API smoke tests passed." -ForegroundColor Green
exit 0