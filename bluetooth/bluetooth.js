// UUID
const SERVICE_UUID = "d5875408-fa51-4763-a75d-7d33cecebc31";
const RX_CHARACTERISTIC_UUID = "a4f01d8c-a037-43b6-9050-1876a8c23584";
// const TX_CHARACTERISTIC_UUID = "a4f01d8c-a037-43b6-9050-1876a8c23584";

(function() {
   document.querySelector('#fuck').addEventListener("click", fuck)
})()

async function fuck() {
    const ctx = document.querySelector('#mychart')
    const bpms = [0]
    const labels = [0]
    let index = 0
    const myChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: 'bpm',
                data: bpms,
                borderColor: '#E0548E',
                backgroundColor: "#fff"
            }]
        },
        options: {
            legend: {
                display: false
            },
            tooltips: {
                enabled: false
            },
            scales: {
                yAxes: [{
                    display: false
                }],
                xAxes: [{
                    display: false
                }]
            }
        }
    })

    if ("bluetooth" in window.navigator) {
        const bluetooth = window.navigator["bluetooth"];
        // 1. device 情報取得
        const device = await bluetooth.requestDevice({
            acceptAllDevices: true,
            optionalServices: [SERVICE_UUID]
        });

        // 2. GATT Server に接続
        const server = await device.gatt.connect();

        // 3. GATT Server から Service を取得する
        const service = await server.getPrimaryService(SERVICE_UUID);

        // 4. Service から Characteristic を取得する
        const characteristic = await service.getCharacteristic(RX_CHARACTERISTIC_UUID);

        // 5. Characteristic を利用する
        // 5.1. READ
        // if(characteristic.properties.read) {
        //     console.log(await characteristic.readValue());
        // }

        if(characteristic.properties.notify) {
            const handler = (event) => {
                const dataView = event.target.value
                const bpm = (new TextDecoder).decode(dataView.buffer)

                const heart = document.querySelector('.heart')

                const beat = heart.animate([
                    {transform: 'scale(1)'},
                    {transform: 'scale(1.5)'},
                    {transform: 'scale(1)'},
                ], (60 / bpm)*1000)
                beat.addEventListener('finish', () => {
                    heart.style.transform = ''
                })

                const beatBpm = document.querySelector('.beatBpm')
                beatBpm.innerHTML = bpm;
                bpms.push(Number(bpm))
                index += 1;
                labels.push(index)
                myChart.update()
            }
            characteristic.addEventListener("characteristicvaluechanged", handler);
            characteristic.startNotifications();
        }
    }
}


