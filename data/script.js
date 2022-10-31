const noiseThreshold = 2000;

/**
 * Spectrum data is an array of arrays of objects with the following structure:
 * [
 *  [
 *  { freq: 0, value: 0.1 },
 * { freq: 1, value: 0.2 },
 * ]
 * ]
 */
let spectrumData = [];

const spectrumCtx = document.getElementById("spectrum-chart").getContext("2d");
const sampleCtx = document.getElementById("sample-chart").getContext("2d");
const spectrumChart = new Chart(spectrumCtx, {
  type: "bar",
  data: {
    labels: [],
    datasets: [
      {
        label: "Spectrum",
        data: [],
        backgroundColor: ["rgba(255, 99, 132, 0.2)"],
        borderColor: ["rgba(255, 99, 132, 1)"],
        borderWidth: 1,
      },
    ],
  },
  options: {
    scales: {
      yAxes: [
        {
          ticks: {
            beginAtZero: true,
          },
        },
      ],
    },
  },
});

const sampleChart = new Chart(sampleCtx, {
  type: "line",
  data: {
    labels: [],
    datasets: [
      {
        label: "Sample",
        data: [],
        backgroundColor: ["rgba(255, 99, 132, 0.2)"],
        borderColor: ["rgba(255, 99, 132, 1)"],
        borderWidth: 1,
      },
    ],
  },
  options: {
    scales: {
      // Range from 0 to 3.3V on the y-axis
      yAxes: [
        {
          ticks: {
            min: 0,
            mx: 3.3,
          },
        },
      ],
    },
  },
});

// setInterval(async () => {
//   const response = await fetch('http://192.168.178.33:80/fft');
//   const eventData = await response.json();
//   setData(eventData);
// })

// Connect to the server using websockets
const socket = new WebSocket("ws://192.168.178.33:80/ws");
socket.addEventListener("message", (event) => {
  const eventData = JSON.parse(event.data);
  setData(eventData);
});

let updateCount = 0;

function setData({ fft, samples }) {
  spectrumData.push([fft]);

  if (spectrumData.length >= 10) {
    console.log("Updating chart", spectrumData);
    // Average the last 10 samples
    const averagedSpectrum = [];
    for (let i = 0; i < fft.length; i++) {
      const values = spectrumData.map((sample) => sample[0][i].value);
      const average = values.reduce((a, b) => a + b, 0) / values.length;
      averagedSpectrum.push({ freq: fft[i].freq, value: average });
    }

    console.log(averagedSpectrum);
    // Update the chart
    spectrumChart.data.labels = averagedSpectrum.map((d) => d.freq);
    spectrumChart.data.datasets[0].data = averagedSpectrum.map((d) => d.value);
    spectrumChart.update("none");
    spectrumData = [];
  }

  if (updateCount % 2 === 0) {
    sampleChart.data = {
      labels: samples.length > 0 ? samples.map((d, i) => i) : [],
      datasets: [
        {
          label: "Sample",
          data: samples.length > 0 ? samples.map((d) => d) : [],
          backgroundColor: ["rgba(255, 99, 132, 0.2)"],
          borderColor: ["rgba(255, 99, 132, 1)"],
          borderWidth: 1,
        },
      ],
    };

    updateCount = 0;
    sampleChart.update("none");
  }
  updateCount++;
}
