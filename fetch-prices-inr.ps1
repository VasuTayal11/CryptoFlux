# fetch-prices-inr.ps1

$response = Invoke-RestMethod -Uri "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum,solana,ripple,dogecoin,cardano,litecoin,polkadot&vs_currencies=inr"

# Save to CSV
"Name,Price (INR)" | Out-File -Encoding UTF8 prices.csv
"Bitcoin,$($response.bitcoin.inr)" | Out-File -Append -Encoding UTF8 prices.csv
"Ethereum,$($response.ethereum.inr)" | Out-File -Append -Encoding UTF8 prices.csv
"Solana,$($response.solana.inr)" | Out-File -Append -Encoding UTF8 prices.csv
"Ripple,$($response.ripple.inr)" | Out-File -Append -Encoding UTF8 prices.csv
"Dogecoin,$($response.dogecoin.inr)" | Out-File -Append -Encoding UTF8 prices.csv
"Cardano,$($response.cardano.inr)" | Out-File -Append -Encoding UTF8 prices.csv
"Litecoin,$($response.litecoin.inr)" | Out-File -Append -Encoding UTF8 prices.csv
"Polkadot,$($response.polkadot.inr)" | Out-File -Append -Encoding UTF8 prices.csv

Write-Host "✅ prices.csv updated successfully."


# Define parameters
$cryptos = "bitcoin,ethereum,solana,ripple,dogecoin,cardano,litecoin,polkadot"
$vsCurrency = "inr"
$days = 30
$outputFile = "prices_30days_minmax.csv"

# Always overwrite file and write header
"Date,Coin,MinPrice,MaxPrice" | Out-File -Encoding UTF8 $outputFile

# Loop through each cryptocurrency
foreach ($crypto in $cryptos.Split(',')) {
    $uri = "https://api.coingecko.com/api/v3/coins/$crypto/market_chart?vs_currency=$vsCurrency&days=$days"
    $response = Invoke-RestMethod -Uri $uri

    # Group prices by date
    $groupedPrices = @{}
    foreach ($pricePoint in $response.prices) {
        $timestampMs = $pricePoint[0]
        $price = $pricePoint[1]
        $date = (Get-Date -Date ([System.DateTimeOffset]::FromUnixTimeMilliseconds($timestampMs).DateTime)).ToString("yyyy-MM-dd")

        if (-not $groupedPrices.ContainsKey($date)) {
            $groupedPrices[$date] = @()
        }
        $groupedPrices[$date] += $price
    }

    # Write min/max prices for each day
    foreach ($date in $groupedPrices.Keys | Sort-Object) {
        $prices = $groupedPrices[$date]
        $minPrice = [Math]::Round(($prices | Measure-Object -Minimum).Minimum, 2)
        $maxPrice = [Math]::Round(($prices | Measure-Object -Maximum).Maximum, 2)
        "$date,$crypto,$minPrice,$maxPrice" | Out-File -Append -Encoding UTF8 $outputFile
    }
}

Write-Host "✅ prices_30days_minmax.csv updated successfully with daily min/max data."

$cryptos = @("BTC", "ETH", "SOL", "XRP", "DOGE", "ADA", "LTC", "DOT")
$data = @()

foreach ($from in $cryptos) {
    $toList = ($cryptos | Where-Object { $_ -ne $from }) -join ","
    $uri = "https://min-api.cryptocompare.com/data/price?fsym=$from&tsyms=$toList"

    try {
        $response = Invoke-RestMethod -Uri $uri -Method Get

        foreach ($to in $response.PSObject.Properties) {
            $data += [PSCustomObject]@{
                Timestamp      = (Get-Date).ToString("s")
                FromCurrency   = $from
                ToCurrency     = $to.Name
                ConversionRate = $to.Value
            }
        }

        # Respect API pacing to avoid soft limits
        Start-Sleep -Milliseconds 800
    } catch {
        Write-Host "❌ Error converting $from to others: $_"
    }
}

$csvFile = "crypto_pairwise_rates_cryptocompare.csv"
$data | Export-Csv -Path $csvFile -NoTypeInformation
Write-Host "✅ Pairwise conversion rates saved using CryptoCompare"
